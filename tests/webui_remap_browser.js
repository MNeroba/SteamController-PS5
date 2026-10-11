/* Playwright CLI function, no product dependency. Requires webui_preview.py.
   Run: playwright-cli run-code --filename tests/webui_remap_browser.js
   The only API here is a loopback RAM fixture; this is not PS5 runtime proof. */
async (page) => {
  page = await page.context().newPage(); // Fresh document; never discard a user's open page.
  const passed = [];
  const check = (ok, name) => { if (!ok) throw new Error(name); passed.push(name); };
  const waitForFunction=page.waitForFunction.bind(page);
  page.waitForFunction=async(...args)=>{try{return await waitForFunction(...args)}catch(error){throw new Error(error.message+"; last check: "+passed.at(-1)+"; predicate: "+args[0])}};
  const value = k => page.evaluate(k => cur[k], k);
  const source = k => page.locator(`#steamStage svg:not([hidden]) .ctl[data-k="${k}"]`);
  const view = (name, v) => page.locator(`.viewswitch[data-controller="${name}"] button[data-view="${v}"]`).click();
  const selected = async k => (await page.locator(`#dualStage .destination[data-out="${k}"].assigned`).count()) > 0;
  const open = async k => { await source(k).click(); await page.locator('#sheet.show').waitFor(); };
  const picker = k => page.locator('#pick .g-' + k);
  const errors = [];
  page.on('pageerror', error => errors.push(error.message));
  await page.addInitScript(() => sessionStorage.setItem('pb.webuiToken', 'preview-only'));
  await page.setViewportSize({width:1440, height:1050});
  await page.goto('http://127.0.0.1:8765/#remap/profile=default/buttons');
  await page.waitForFunction(() => saved !== '' && profiles.length === 2);
  const base = await page.evaluate(() => 'name=Default\n' + Object.entries(DEFAULTS).map(([k,v])=>k+'='+v).join('\n') + '\nSHIFT=L4\nDEADZONE=9\n' + BITS.map(k=>['long','double','shift'].map(a=>k+'.'+a+'=NONE\n').join('')+k+'.turbo=0\n'+k+'.toggle=0\n').join('')+'LT.full=NONE\nRT.full=NONE\n');
  await page.request.post('http://127.0.0.1:8765/api/profile?id=default', {headers:{Authorization:'Bearer preview-only'},data:base});
  await page.reload();await page.waitForFunction(() => saved !== '' && cur.A === 'CROSS');
  // Audit regressions: hostile profiles never reach HTML attributes or prototypes.
  const auditFailures=[];
  const auditCheck=(ok,name)=>{if(ok)passed.push(name);else auditFailures.push(name)};
  const badImports=await page.evaluate(()=>{
    const before=fmt(cur),bad=[
      'A=CROSS" data-audit-injected="yes', 'A=CROSS+UNKNOWN',
      'UNKNOWN=CROSS\nA=CROSS', 'A.full=PS', 'SHIFT=QAM\nA=CROSS',
      '__proto__=CROSS\nA=CROSS', 'A.long=PS" autofocus="yes',
      'A.=CROSS', 'name=bad\0name\nA=CROSS'
    ];
    return bad.map(text=>{try{const next=mergeImportedProfile(cur,parse(text));cur=next;render();return false}
      catch(e){return fmt(cur)===before}finally{cur=parse(before);render()}});
  });
  badImports.forEach((ok,i)=>auditCheck(ok,'reject invalid imported identifier '+i));
  const domSafe=await page.evaluate(()=>{
    const original=cur.A;cur.A='CROSS" data-audit-injected="yes';render();
    const safe=!document.querySelector('[data-audit-injected]');cur.A=original;render();return safe;
  });
  auditCheck(domSafe,'rendering cannot create attributes from a corrupt binding');
  await view('steam','back');await source('L4').click();
  auditCheck(!await selected('L3'),'Shift modifier has no destination highlight');
  await view('steam','front');
  const flags=await page.evaluate(()=>{
    const a=mergeImportedProfile(cur,parse('A.turbo=1\nA.toggle=1\n'));
    const b=mergeImportedProfile(cur,parse('A.toggle=1\nA.turbo=1\n'));
    return [a['A.turbo']!=='1'&&a['A.toggle']==='1',b['A.turbo']!=='1'&&b['A.toggle']==='1'];
  });
  flags.forEach((ok,i)=>auditCheck(ok,'import Turbo/Toggle conflict normalized independent of order '+i));
  // Fixture saves through production sc2_profile_parse/format on the host.
  let profileGets=0;
  await page.route('**/api/profile?id=default',async route=>{
    if(route.request().method()==='GET'){
      profileGets++;await route.continue();
    }else await route.continue();
  });
  await page.evaluate(()=>{cur.DEADZONE='999';render()});
  await page.locator('#save').click();await page.waitForFunction(()=>!isDirty());
  auditCheck(profileGets>0&&await value('DEADZONE')==='40','Save reads and displays server canonical state');
  await page.unroute('**/api/profile?id=default');
  await page.evaluate(()=>{window.confirm=()=>true;cur.A='CROSS';render()});
  const importBefore=await page.evaluate(()=>fmt(cur));
  await page.locator('#impFile').setInputFiles({name:'hostile.ini',mimeType:'text/plain',buffer:Buffer.from('A=CROSS" data-audit-injected="yes\n')});
  await page.waitForFunction(()=>document.querySelector('#toast').textContent.startsWith('Import rejected:'));
  auditCheck(await page.evaluate(()=>fmt(cur))===importBefore&&!await page.locator('[data-audit-injected]').count(),'file import rejected atomically before rendering');
  // Successful POST plus failed GET must retain the unsaved draft and error.
  await page.route('**/api/profile?id=default',async route=>{
    if(route.request().method()==='GET')await route.fulfill({status:500,body:'injected readback failure'});
    else await route.continue();
  });
  await page.evaluate(()=>{cur.DEADZONE='22';render()});
  await page.locator('#save').click();await page.waitForFunction(()=>!saving);
  auditCheck(await page.evaluate(()=>isDirty()&&cur.DEADZONE==='22')&&(await page.locator('#toast').textContent()).startsWith('Save not confirmed:'),'failed readback cannot report Saved or discard draft');
  await page.unroute('**/api/profile?id=default');
  await page.route('**/api/profile?id=default',async route=>{
    if(route.request().method()==='GET')await route.fulfill({status:200,body:'name=truncated\nA=CROSS\n'});
    else await route.continue();
  });
  await page.locator('#save').click();await page.waitForFunction(()=>!saving);
  auditCheck(await page.evaluate(()=>isDirty()&&cur.DEADZONE==='22')&&(await page.locator('#toast').textContent()).includes('Incomplete profile'),'incomplete canonical readback is rejected without overwriting draft');
  await page.unroute('**/api/profile?id=default');
  // Simulate a user editing while an actual canonical GET is pending.
  let releaseGet;const getGate=new Promise(resolve=>releaseGet=resolve);
  let getStarted;const getReady=new Promise(resolve=>getStarted=resolve);
  await page.route('**/api/profile?id=default',async route=>{
    if(route.request().method()==='GET'){getStarted();await getGate;await route.continue()}
    else await route.continue();
  });
  await page.evaluate(()=>{cur.DEADZONE='999';render()});await page.locator('#save').click();await getReady;
  await page.evaluate(()=>{cur.A='CIRCLE';render()});releaseGet();await page.waitForFunction(()=>!saving);
  auditCheck(await page.evaluate(()=>cur.A==='CIRCLE'&&isDirty()&&parse(saved).DEADZONE==='40'),'pending Save preserves newer edits and tracks the canonical saved snapshot');
  await page.unroute('**/api/profile?id=default');
  // A late save result for Default must not replace another opened profile.
  let releaseOther;const otherGate=new Promise(resolve=>releaseOther=resolve);
  let otherStarted;const otherReady=new Promise(resolve=>otherStarted=resolve);
  await page.route('**/api/profile?id=default',async route=>{
    if(route.request().method()==='GET'){otherStarted();await otherGate;await route.continue()}
    else await route.continue();
  });
  await page.locator('#save').click();await otherReady;
  await page.evaluate(()=>load('PPSA00001',true));releaseOther();await page.waitForFunction(()=>!saving);
  auditCheck(await page.evaluate(()=>editing==='PPSA00001'&&cur.A==='SQUARE'&&!isDirty()),'late save readback cannot replace a different profile');
  await page.unroute('**/api/profile?id=default');await page.evaluate(()=>load('default',true));
  // Production POST merges into the previous profile. Missing optional fields
  // must be transmitted as explicit tombstones to persist an editor clear.
  await page.evaluate(()=>{cur['B.long']='PS';cur['B.toggle']='1';cur['B.turbo']='1';cur['LT.full']='L1';render()});
  await page.locator('#save').click();await page.waitForFunction(()=>!saving);
  auditCheck(await page.evaluate(()=>cur['B.toggle']==='1'&&cur['B.turbo']!=='1'),'Save applies the shared Turbo/Toggle validation');
  await page.locator('#impFile').setInputFiles({name:'clear.ini',mimeType:'text/plain',buffer:Buffer.from('B=CIRCLE\nB.long=NONE\nB.toggle=0\nB.turbo=0\nLT.full=NONE\n')});
  await page.waitForFunction(()=>cur['B.long']===undefined);
  await page.locator('#save').click();await page.waitForFunction(()=>!saving);
  auditCheck(await page.evaluate(()=>!cur['B.long']&&!cur['B.toggle']&&!cur['B.turbo']&&!cur['LT.full']),'Save persists cleared activators and flags through the production merge parser');
  // Real touch event at the former L1/L2 overlap, rather than hotspot centers.
  const auditTouch=await page.context().browser().newContext({viewport:{width:390,height:844},isMobile:true,hasTouch:true});
  try{
    await auditTouch.addInitScript(()=>sessionStorage.setItem('pb.webuiToken','preview-only'));
    const tp=await auditTouch.newPage();await tp.goto('http://127.0.0.1:8765/#remap/profile=default/buttons');
    await tp.waitForFunction(()=>saved!=='');await tp.locator('#steamStage svg:not([hidden]) .ctl[data-k="A"]').tap();await tp.locator('#cancel').tap();
    await tp.locator('#dualStage').scrollIntoViewIfNeeded();
    for(const width of [390,768]){
      await tp.setViewportSize({width,height:844});
      for(const [v,points] of [
        ['front',[[262,262,'L1'],[260,288,'L2'],[260,306,'L2'],[990,262,'R1'],[994,288,'R2'],[994,306,'R2']]],
        ['back',[[980,300,'L1'],[975,380,'L2'],[274,300,'R1'],[279,380,'R2']]]
      ]){
        await tp.locator('.viewswitch[data-controller="dual"] [data-view="'+v+'"]').tap();
        await tp.locator('#dualStage').scrollIntoViewIfNeeded();
        const overlap=await tp.evaluate(()=>{
          const svg=document.querySelector('#dualStage svg:not([hidden])'),hits=[...svg.querySelectorAll('.destination')];
          const shapes=hits.map(g=>({key:g.dataset.out,shape:g.querySelector('.s'),box:g.querySelector('.s').getBBox()}));
          for(let x=150;x<=1120;x+=2)for(let y=230;y<=690;y+=2){
            const point=new DOMPoint(x,y).matrixTransform(svg.getScreenCTM());
            const inside=shapes.filter(({shape,box})=>{
              const p=point.matrixTransform(shape.getScreenCTM().inverse());
              return p.x>=box.x&&p.x<=box.x+box.width&&p.y>=box.y&&p.y<=box.y+box.height&&shape.isPointInFill(p);
            });
            if(inside.length>1)return inside.map(s=>s.key).join('/');
          }return null;
        });
        auditCheck(!overlap,'all output fills disjoint across their full sampled geometry '+v+':'+width);
        for(const [x,y,key] of points){
          const point=await tp.evaluate(([x,y])=>new DOMPoint(x,y).matrixTransform(document.querySelector('#dualStage svg:not([hidden])').getScreenCTM()).toJSON(),[x,y]);
          await tp.touchscreen.tap(point.x,point.y);
          auditCheck(await tp.evaluate(key=>sheetKey==='A'&&draft[''].has(key),key),'touch '+v+':'+width+' at '+x+','+y+' chooses '+key);
          if(await tp.locator('#sheet.show').count())await tp.locator('#cancel').tap();
        }
      }
    }
  }finally{await auditTouch.close()}
  if(auditFailures.length)throw new Error('Audit regressions: '+auditFailures.join('; '));
  await page.request.post('http://127.0.0.1:8765/api/profile?id=default',{headers:{Authorization:'Bearer preview-only'},data:base});
  await page.reload();await page.waitForFunction(()=>saved!==''&&cur.A==='CROSS');
  const calibrated = await page.evaluate(() => {
    const front=Object.fromEntries(SHAPES.dual.front.shapes.map(s=>[s.id,s]));
    const back=Object.fromEntries(SHAPES.dual.back.shapes.map(s=>[s.id,s]));
    const cy=s=>s.pts.reduce((n,p)=>n+p[1],0)/s.pts.length;
    return {frontL1:front.l1.cy,frontL2:front.l2.cy,frontR1:front.r1.cy,frontR2:front.r2.cy,
      tri:[front.tri.cx,front.tri.cy],circle:[front.ci.cx,front.ci.cy],
      backL1:cy(back.l1),backL2:cy(back.l2),backR1:cy(back.r1),backR2:cy(back.r2)};
  });
  check(calibrated.frontL1<calibrated.frontL2&&calibrated.frontR1<calibrated.frontR2,'front L1/R1 target upper bumper buttons');
  check(calibrated.backL1<calibrated.backL2&&calibrated.backR1<calibrated.backR2,'back L1/R1 target upper bumper buttons');
  check(calibrated.tri[0]===994&&calibrated.tri[1]===360&&calibrated.circle[0]===1078&&calibrated.circle[1]===440,'face-button overlays centered on source artwork');
  const decoded = await page.evaluate(async () => {
    const result = [];
    for (const device of ['steam', 'dual']) for (const view of ['front', 'back']) {
      const v = SHAPES[device][view], image = new Image();
      image.src = v.img; await image.decode();
      result.push({device,view,width:image.naturalWidth,height:image.naturalHeight,iw:v.iw,ih:v.ih});
    }
    return result;
  });
  check(decoded.length === 4 && decoded.every(i => i.width > 0 && i.height > 0), 'four embedded WebPs decode');
  check(decoded.some(i => i.width !== i.iw || i.height !== i.ih), 'logical reference dimensions preserved independently of pixels');
  check(await page.locator('#steamStage svg:not([hidden])').count() === 1, 'one source view visible');
  check(await page.locator('#dualStage svg:not([hidden])').count() === 1, 'one output view visible');
  check(await page.locator('.assignment-list .row').count() === 24, 'all 24 inputs listed');
  await view('steam','back');
  await source('L4').click();
  check(await page.locator('#sheet.show').count()===0&&await page.locator('.assignment-list .row[data-k="L4"]').textContent().then(t=>t.includes('Shift modifier')&&!t.includes('L3')),'Shift input shown only as a modifier, with no phantom action');
  check((await page.locator('#toast').textContent()).includes('choose another Shift button'),'Shift modifier explains why its editor is unavailable');
  await view('steam','front');
  await source('A').click();await page.locator('#sheet.show').waitFor();
  const outputGroups=await page.locator('#pick .pick-group h4').allTextContents();
  check(outputGroups.join('|')==='Face buttons|D-pad|Shoulders & triggers|Sticks|System|Touchpad|SteamController-PS5 actions','output actions grouped by function');
  check((await picker('CROSS').innerText()).trim()==='✕'&&await picker('CROSS').getAttribute('aria-label')==='Cross (✕)','face labels use symbols visually and descriptive accessible names');
  await page.locator('#cancel').click();
  check((await page.locator('#dualHint').textContent()).includes('A selected'),'DualSense instructions follow the selected Steam input');
  await page.locator('#dualStage .destination[data-out="CIRCLE"]').click();
  check(await page.locator('#sheet.show').count()===1&&await page.evaluate(()=>sheetKey==='A'&&draft[''].has('CIRCLE')),'DualSense tap stages the selected output for confirmation');
  check((await page.locator('#actHelp').textContent()).includes('staged; choose Done to apply or Cancel to discard'),'direct output assignment explains the confirmation step');
  await page.locator('#cancel').click();
  check(await value('A')==='CROSS','Cancel discards a staged DualSense output');
  await page.evaluate(()=>{cur.B='CIRCLE';cur['A.long']='PS';cur['B.double']='R1';cur.DEADZONE='23';render();window.confirm=()=>true});
  await page.locator('#impFile').setInputFiles({name:'partial.ini',mimeType:'text/plain',buffer:Buffer.from('A=SQUARE\nA.long=NONE\n')});
  await page.waitForFunction(()=>cur.A==='SQUARE'&&cur['A.long']===undefined);
  check(await value('B')==='CIRCLE'&&await value('B.double')==='R1'&&await value('DEADZONE')==='23','profile import merges supplied keys and preserves omitted settings');
  check(!await page.locator('.assignment-list .row[data-k="A"]').textContent().then(t=>t.includes('Long:')),'NONE activator is normalized and not shown as a phantom');
  await page.evaluate(()=>{cur=parse(saved);render()});
  await page.reload();await page.waitForFunction(()=>saved!==''&&cur.A==='CROSS');
  await page.evaluate(()=>{cur.SHIFT='NONE';render()});
  // Every remappable input must open the production editor at its physical hit area.
  for (const v of ['front','back']) {
    await view('steam', v);
    const keys = await page.locator('#steamStage svg:not([hidden]) .ctl').evaluateAll(gs => gs.map(g => g.dataset.k));
    for (const k of keys) {
      if (k === 'QAM') continue;
      await open(k);
      check(await page.evaluate(k => sheetKey === k, k), 'physical source hotspot ' + v + ':' + k);
      await page.locator('#cancel').click();
    }
  }
  await view('steam', 'front');
  check(await source('QAM').getAttribute('aria-disabled') === 'true', 'QAM marked non-remappable');
  const qam = await source('QAM').boundingBox();
  await page.mouse.click(qam.x + qam.width / 2, qam.y + qam.height / 2);
  check(await page.locator('#sheet.show').count() === 0 && await value('QAM') === 'NONE', 'QAM reserved, never enters editor');
  await open('A');
  check(await selected('CROSS'), 'current destination highlighted on open');
  await page.locator('#clear').click();
  for (const k of ['CROSS','CIRCLE','SQUARE','TRIANGLE','L1','R1','L2','R2','L3','R3','UP','DOWN','LEFT','RIGHT','OPTIONS','CREATE','PS','TOUCHPAD']) {
    await picker(k).click();
    check(await selected(k), 'output highlighted ' + k);
    await picker(k).click();
  }
  for(const v of ['front','back']){
    await view('sheet',v);
    const keys = await page.locator('#sheetDual svg:not([hidden]) .destination').evaluateAll(gs=>gs.map(g=>g.dataset.out));
    for(const k of keys){
      const spot = page.locator(`#sheetDual svg:not([hidden]) .destination[data-out="${k}"]`);
      await spot.click();check(await page.evaluate(k=>draft[act].has(k),k),'physical output hotspot '+v+':'+k);
      await spot.click();
    }
  }
  await view('sheet','front');
  await picker('L1').click(); await picker('R1').click();
  check(await selected('L1') && await selected('R1'), 'multi output draft highlights both shoulders');
  await page.locator('#cancel').click();
  check(await value('A') === 'CROSS' && await selected('CROSS') && !await selected('L1'), 'Cancel restores saved assignment and highlight');
  await page.locator('#shiftSel').selectOption('L4');
  await open('A'); await page.locator('#clear').click();
  await picker('L1').click(); await picker('R1').click();
  await page.locator('#actTabs [data-a=".long"]').click(); await picker('PS').click();
  await page.locator('#actTabs [data-a=".double"]').click(); await picker('CIRCLE').click();
  await page.locator('#actTabs [data-a=".shift"]').click(); await picker('TOUCHPAD').click();
  await page.locator('#actTabs [data-a=""]').click();
  await page.locator('#optTurbo').check(); await page.locator('#optToggle').check();
  check(!await page.locator('#optTurbo').isChecked(), 'Turbo / Toggle mutual exclusion preserved');
  await page.locator('#done').click();
  check(await value('A') === 'L1+R1' && await selected('L1') && await selected('R1'), 'Done applies combo with persistent highlight');
  check(await value('A.long') === 'PS' && await value('A.double') === 'CIRCLE' && await value('A.shift') === 'TOUCHPAD' && await value('A.toggle') === '1', 'Long Double Shift Toggle preserved');
  await view('steam', 'back'); await open('LT');
  await page.locator('#actTabs [data-a=".full"]').click(); await picker('L1').click();
  await page.locator('#done').click(); check(await value('LT.full') === 'L1', 'Full Pull preserved');
  await open('L5'); await page.locator('#cancel').click();
  check(await selected('L1') && await selected('R1'), 'back grip combo maps shared output IDs');
  await view('dual', 'back');
  const back = await page.locator('#dualStage svg:not([hidden]) .destination').evaluateAll(gs => gs.map(g => ({key:g.dataset.out,x:g.getBBox().x})));
  check(back.find(g=>g.key==='L2').x > back.find(g=>g.key==='R2').x, 'back-view physical left / right correct');
  await view('steam','front');await view('dual','front');
  // Keyboard opens editor and Escape preserves bindings and returns focus.
  await source('L3').focus(); await page.keyboard.press('Enter');
  await page.locator('#sheet.show').waitFor();
  await page.keyboard.press('Escape');
  check(await page.evaluate(() => document.activeElement.dataset.k === 'L3'), 'keyboard Enter Escape and focus return');
  await source('R3').focus();await page.keyboard.press('Space');await page.locator('#sheet.show').waitFor();
  await page.locator('#done').focus();await page.keyboard.press('Tab');
  check(await page.evaluate(()=>document.activeElement.closest('#sheet')!==null),'modal Tab focus stays inside dialog');
  await page.keyboard.press('Escape');
  check(await value('L3') === 'L3' && await value('R3') === 'R3', 'stick clicks retain independent digital IDs');
  await source('A').hover();
  check(await page.locator('#steamHint').textContent() === 'A → L1 + R1', 'hover hint preserves selected mapping ribbon');
  const response = page.waitForResponse(r => r.url().includes('/api/profile?') && r.request().method() === 'POST');
  await page.locator('#save').click();const r = await response;
  check(r.status() === 200 && r.request().headers().authorization === 'Bearer preview-only' && r.request().postData().includes('A=L1+R1'), 'profile save uses existing authenticated API and INI');
  await page.waitForFunction(() => !isDirty());
  await page.reload();await page.waitForFunction(() => saved !== '');
  check(await value('A') === 'L1+R1' && await value('LT.full') === 'L1', 'save reload retains bindings and activators');
  await page.locator('#prof').selectOption('PPSA00001'); await page.waitForFunction(() => editing === 'PPSA00001' && cur.A === 'SQUARE');
  await open('A'); await page.locator('#cancel').click();
  check(await selected('SQUARE') && !await selected('L1'), 'profile load refreshes mapping and highlights');
  await page.locator('#prof').selectOption('default'); await page.waitForFunction(() => editing === 'default' && cur.A === 'L1+R1');
  await view('steam','back'); await open('L5'); await page.locator('#cancel').click();
  await view('dual','back');
  await page.locator('#sec-buttons').scrollIntoViewIfNeeded();
  await page.screenshot({path:'output/playwright/remap-desktop-back.png'});
  await view('steam','front');await view('dual','front');
  await open('A');await page.locator('#cancel').click();
  await page.locator('#sec-buttons').scrollIntoViewIfNeeded();
  await page.screenshot({path:'output/playwright/remap-desktop.png'});
  await open('A'); await page.screenshot({path:'output/playwright/remap-editor.png'}); await page.locator('#cancel').click();
  // Two screen widths, actual layout metrics and same SVG coordinate transforms.
  for (const width of [768,390]) {
    await page.setViewportSize({width,height:1100});await page.locator('#sec-buttons').scrollIntoViewIfNeeded();
    const layout = await page.locator('.controller-pair').evaluate(el => {
      const [a,b] = [...el.children].map(e=>e.getBoundingClientRect());
      return {ax:a.x,ay:a.y,bx:b.x,by:b.y,overflow:document.documentElement.scrollWidth>innerWidth};
    });
    check(!layout.overflow, 'no horizontal overflow at ' + width + ': ' + JSON.stringify(layout));
    if(width===768){
      // Wider system font metrics must not push navigation/status off screen.
      await page.locator('header').evaluate(e=>e.style.fontSize='24px');
      const head=await page.locator('.head').evaluate(e=>({width:innerWidth,scroll:document.documentElement.scrollWidth,items:[...e.children].map(c=>{const r=c.getBoundingClientRect();return {left:r.left,right:r.right}})}));
      check(head.scroll<=head.width,'wide-font header has no overflow: '+JSON.stringify(head));
      check(head.items.every(r=>r.left>=0&&r.right<=head.width),'all header controls remain reachable with wide fonts');
      await page.locator('header').evaluate(e=>e.style.fontSize='');
    }
    if(width===390)check(layout.by > layout.ay && Math.abs(layout.ax-layout.bx)<1, 'mobile controllers stack vertically');
    await open('A');check(await page.evaluate(()=>sheetKey==='A'), 'aligned clickable hotspot after resize ' + width);await page.locator('#cancel').click();
  }
  await page.locator('.controller-workspace').screenshot({path:'output/playwright/remap-mobile.png'});
  const touchContext = await page.context().browser().newContext({viewport:{width:390,height:844},isMobile:true,hasTouch:true,deviceScaleFactor:2});
  try{
    await touchContext.addInitScript(()=>sessionStorage.setItem('pb.webuiToken','preview-only'));
    const touchPage = await touchContext.newPage();
    await touchPage.goto('http://127.0.0.1:8765/#remap/profile=default/buttons');await touchPage.waitForFunction(()=>saved!=='');
    check(await touchPage.evaluate(()=>matchMedia('(pointer: coarse)').matches),'touch browser exposes coarse-pointer targets');
    await touchPage.locator('#steamStage svg:not([hidden]) .ctl[data-k="A"]').tap();
    check(await touchPage.locator('#sheet.show').count()===1,'touch tap opens production editor');
    await touchPage.waitForFunction(()=>document.querySelector('#sheetScrollHint').textContent.includes('below'));
    check(await touchPage.locator('#sheetScrollHint').isVisible()&&(await touchPage.locator('#sheetScrollHint').textContent()).includes('below'),'mobile editor indicates more output groups below');
    await touchPage.locator('#pick .g-PB_GYRO_TOGGLE').scrollIntoViewIfNeeded();
    check(await touchPage.locator('#pick .g-PB_GYRO_TOGGLE').isVisible(),'last internal action reachable in mobile dialog');
    await touchPage.locator('#pick .g-PB_GYRO_TOGGLE').tap();
    check(await touchPage.evaluate(()=>draft[''].has('PB_GYRO_TOGGLE')),'last mobile output action selectable');
    await touchPage.locator('#pick .g-CROSS').tap();await touchPage.locator('#cancel').tap();
    check(await touchPage.evaluate(()=>cur.A==='L1+R1'),'touch Cancel retains assignment');
    await touchPage.locator('#dualStage').scrollIntoViewIfNeeded();
    const circleBox=await touchPage.locator('#dualStage svg:not([hidden]) .destination[data-out="CIRCLE"]').boundingBox();
    await touchPage.touchscreen.tap(circleBox.x+circleBox.width+13,circleBox.y+circleBox.height/2);
    const touchAssignment=await touchPage.evaluate(()=>({coarse:matchMedia('(pointer: coarse)').matches,selectedKey,sheetKey,act,circleStaged:draft?.['']?.has('CIRCLE')||false,rect:document.querySelector('#dualStage svg:not([hidden]) .destination[data-out="CIRCLE"]').getBoundingClientRect().toJSON()}));
    check(await touchPage.locator('#sheet.show').count()===1&&touchAssignment.circleStaged,'enlarged touch target stages a nearby DualSense output '+JSON.stringify(touchAssignment));
    await touchPage.locator('#cancel').tap();
    await touchPage.locator('.viewswitch[data-controller="steam"] [data-view="back"]').tap();
    await touchPage.locator('#steamStage svg:not([hidden]) .ctl[data-k="L5"]').tap();
    check(await touchPage.evaluate(()=>sheetKey==='L5'),'touch back grip maps physical left input');
    await touchPage.locator('#cancel').tap();
  }finally{await touchContext.close()}
  await page.setViewportSize({width:1440,height:1050});
  await page.locator('[data-tab="games"]').click();check(await page.locator('#tab-games').isVisible(), 'Games navigation still works');
  await page.locator('[data-tab="help"]').click();check(await page.locator('#tab-help').isVisible(), 'Settings navigation still works');
  check(await page.evaluate(()=>{
    try{serverProfile(profileWrite(cur).replace(/^MOTION=.*\n/m,''));return false}catch(e){return e.message.includes('Incomplete')}
  }),'canonical readback requires settings as well as all base bindings');
  const originalName=await page.evaluate(()=>cur.name);
  check(await page.evaluate(async()=>{cur.name='N'.repeat(60);render();return !await saveCurrent()&&!saving&&savePromise===null&&isDirty()}),'invalid Save returns failure and releases the shared Save operation');
  await page.evaluate(name=>{cur.name=name;cur.A='CROSS';render()},originalName);
  check(await page.evaluate(async()=>await saveCurrent()&&!saving&&savePromise===null&&!isDirty()),'valid Save succeeds after a validation failure');
  // Profile-operation regressions. Gates control actual browser responses.
  await page.evaluate(()=>{switchTab('remap')});
  const gateProfile=async(id,method='GET')=>{
    let release,ready;const active=[];
    const gate=new Promise(r=>release=r),started=new Promise(r=>ready=r);
    const pattern='**/api/'+(method==='DELETE'?'delete':'profile')+'?id='+id;
    await page.route(pattern,async route=>{
      if(route.request().method()!==(method==='DELETE'?'POST':method))return route.continue();
      let done;active.push(new Promise(r=>done=r));
      try{const response=await route.fetch();ready();await gate;await route.fulfill({response})}finally{done()}
    });
    return {started,release,remove:async()=>{await Promise.all(active);await page.unroute(pattern)}};
  };
  await page.evaluate(()=>load('default',true));
  const oldLoad=await gateProfile('PPSA00001');
  await page.evaluate(()=>{window.navOld=load('PPSA00001',true)});await oldLoad.started;
  await page.evaluate(()=>load('default',true));oldLoad.release();await page.evaluate(()=>window.navOld);await oldLoad.remove();
  check(await page.evaluate(()=>editing==='default'&&document.querySelector('#prof').value==='default'&&location.hash.includes('profile=default')),'reverse Load responses retain latest committed selection and hash');
  const draftLoad=await gateProfile('PPSA00001');
  await page.evaluate(()=>{window.navDraft=load('PPSA00001',true)});await draftLoad.started;
  await page.evaluate(()=>{cur.A='TRIANGLE';render()});draftLoad.release();await page.evaluate(()=>window.navDraft);await draftLoad.remove();
  check(await page.evaluate(()=>editing==='default'&&cur.A==='TRIANGLE'&&isDirty()),'late Load cannot overwrite a newer draft');
  await page.route('**/api/profile?id=PPSA00001',r=>r.fulfill({status:500,body:'injected load failure'}));
  await page.evaluate(()=>{window.navFailed=load('PPSA00001')});await page.locator('#profileGuard[open]').waitFor();
  await page.locator('#guardDiscard').click();await page.evaluate(()=>window.navFailed);
  check(await page.evaluate(()=>editing==='default'&&cur.A==='TRIANGLE'&&isDirty()&&document.querySelector('#prof').value==='default'),'failed authorized Load preserves dirty draft and selection');
  await page.unroute('**/api/profile?id=PPSA00001');
  await page.evaluate(()=>{window.navCancelled=load('PPSA00001')});await page.locator('#profileGuard[open]').waitFor();await page.keyboard.press('Escape');
  check(await page.evaluate(async()=>!await window.navCancelled&&editing==='default'&&cur.A==='TRIANGLE'),'Escape cancels profile navigation without changes');
  // Save failure through the explicit leave guard.
  await page.route('**/api/profile?id=default',async route=>route.request().method()==='POST'?route.fulfill({status:500,body:'injected save failure'}):route.continue());
  await page.evaluate(()=>{window.navSaveFailed=load('PPSA00001')});await page.locator('#profileGuard[open]').waitFor();await page.locator('#guardSave').click();await page.evaluate(()=>window.navSaveFailed);
  check(await page.evaluate(()=>editing==='default'&&cur.A==='TRIANGLE'&&isDirty()),'failed Save in navigation guard retains draft');
  await page.unroute('**/api/profile?id=default');
  await page.evaluate(()=>{window.navSaveOK=load('PPSA00001')});await page.locator('#profileGuard[open]').waitFor();await page.locator('#guardSave').click();await page.evaluate(()=>window.navSaveOK);
  check(await page.evaluate(()=>editing==='PPSA00001'&&!isDirty()),'Save and canonical confirmation authorize profile navigation');
  check((await page.request.get('http://127.0.0.1:8765/api/profile?id=default',{headers:{Authorization:'Bearer preview-only'}})).status()===200,'guard Save keeps default profile persisted');
  // A game-list cancel must precede the name prompt and any creation request.
  await page.evaluate(()=>{window.promptCount=0;window.prompt=()=>{window.promptCount++;return 'Created fixture'};cur.A='CIRCLE';render();switchTab('games')});
  await page.locator('#games button[data-id="PPSA00002"]').waitFor();
  await page.locator('#games button[data-id="PPSA00002"]').click();await page.locator('#profileGuard[open]').waitFor();await page.locator('#guardCancel').click();
  check(await page.evaluate(()=>editing==='PPSA00001'&&cur.A==='CIRCLE'&&window.promptCount===0),'game-list creation Cancel preserves draft and never prompts name');
  check((await page.request.get('http://127.0.0.1:8765/api/profile?id=PPSA00002',{headers:{Authorization:'Bearer preview-only'}})).status()===404,'cancelled creation leaves no orphan profile');
  await page.route('**/api/status',r=>r.fulfill({status:200,body:JSON.stringify({connected:false,active:'default',inputs:0,title:'PPSA00002',title_name:'New game fixture'})}));
  await page.evaluate(()=>{status.title='PPSA00002';status.title_name='New game fixture';status.active='default';lastNow='';renderNow();switchTab('remap')});
  await page.locator('#nowAction').click();await page.locator('#profileGuard[open]').waitFor();await page.locator('#guardCancel').click();
  check(await page.evaluate(()=>editing==='PPSA00001'&&cur.A==='CIRCLE'&&window.promptCount===0),'Now Playing shortcut uses the same unsaved-create guard');
  await page.unroute('**/api/status');
  await page.evaluate(()=>{window.navCreate= createFor('PPSA00002','New game')});await page.locator('#profileGuard[open]').waitFor();await page.locator('#guardDiscard').click();await page.evaluate(()=>window.navCreate);
  check(await page.evaluate(()=>editing==='PPSA00002'&&cur.name==='Created fixture'&&!isDirty()&&window.promptCount===1),'Discard authorizes creation and commits the confirmed new profile');
  // Save pending while leaving: the explicit Save choice waits for confirmation.
  await page.evaluate(()=>{cur.A='SQUARE';render()});const pendingSave=await gateProfile('PPSA00002');
  await page.locator('#save').click();await pendingSave.started;
  await page.evaluate(()=>{window.navPendingSave=load('default')});await page.locator('#profileGuard[open]').waitFor();await page.locator('#guardSave').click();
  pendingSave.release();await page.evaluate(()=>window.navPendingSave);await pendingSave.remove();
  check(await page.evaluate(()=>editing==='default'&&!isDirty()),'pending Save can be confirmed before switching profile');
  // Returning to the same ID while old Save is pending must not update its new editor.
  await page.evaluate(()=>{cur.A='CROSS';render()});const saveReturn=await gateProfile('default');
  await page.locator('#save').click();await saveReturn.started;
  await page.evaluate(()=>load('PPSA00001',true));
  // Default GET is gated, so reload it after releasing the old confirmation.
  await page.evaluate(()=>{window.navReturn=load('default',true)});saveReturn.release();await page.evaluate(()=>window.navReturn);await page.waitForFunction(()=>!saving);await saveReturn.remove();
  check(await page.evaluate(()=>editing==='default'&&!isDirty()),'Save and reload of the same ID do not attach stale confirmation to a different editor');
  // Delete completes after a newer selection and draft are committed.
  await page.evaluate(()=>load('PPSA00002',true));const deletion=await gateProfile('PPSA00002','DELETE');
  await page.evaluate(()=>{window.confirm=()=>true;document.querySelector('#del').click()});await deletion.started;
  await page.evaluate(()=>load('default',true));await page.evaluate(()=>{cur.A='CIRCLE';render()});deletion.release();await deletion.remove();
  await page.waitForFunction(()=>!profiles.some(p=>p.id==='PPSA00002'));
  check(await page.evaluate(()=>editing==='default'&&cur.A==='CIRCLE'&&isDirty()),'delayed Delete cannot replace newer selection or draft');
  // Back/Forward invalidates an earlier load even when Back selects the current ID.
  await page.evaluate(()=>load('default',true));const historyLoad=await gateProfile('PPSA00001');
  await page.evaluate(()=>{history.pushState(null,'','#remap/profile=PPSA00001/buttons');dispatchEvent(new PopStateEvent('popstate'))});await historyLoad.started;
  const beforeBack=await page.evaluate(()=>navigationGeneration);
  await page.evaluate(()=>history.back());await page.waitForFunction(g=>navigationGeneration>g,beforeBack);
  await page.waitForFunction(()=>location.hash.includes('profile=default'));
  historyLoad.release();await historyLoad.remove();await page.waitForTimeout(100);
  check(await page.evaluate(()=>editing==='default'&&location.hash.includes('profile=default')),'Back cancels pending profile history load');
  await page.evaluate(()=>history.forward());await page.waitForFunction(()=>editing==='PPSA00001');
  check(await page.evaluate(()=>document.querySelector('#prof').value==='PPSA00001'&&location.hash.includes('profile=PPSA00001')),'Forward commits profile, dropdown and URL together');
  await page.evaluate(()=>load('default',true));
  // First-run presentation: no filesystem access and no token prompt.
  const paired = await page.context().newPage();
  await paired.setViewportSize({width:390,height:844});
  await paired.addInitScript(()=>{sessionStorage.removeItem('pb.webuiToken');window.prompt=()=>{throw new Error('Token prompt forbidden')}});
  await paired.goto('http://127.0.0.1:8765/');
  await paired.locator('#pairing[open]').waitFor();
  check(await paired.locator('#steamStage svg').count()===2,'public interface and controller artwork initialize before pairing');
  check((await paired.locator('#conn').textContent())==='Pairing required','first launch clearly asks for pairing');
  check(await paired.evaluate(()=>document.documentElement.scrollWidth<=390),'pairing screen fits a mobile viewport');
  await paired.locator('#pairCode').fill('00000000');await paired.locator('#pairSubmit').click();
  await paired.waitForFunction(()=>document.querySelector('#pairMessage').textContent.includes('rejected'));
  check(await paired.locator('#pairing[open]').count()===1,'rejected pairing retains integrated screen');
  await paired.locator('#pairCode').fill('12345678');await paired.locator('#pairSubmit').click();
  await paired.waitForFunction(()=>saved!==''&&!document.querySelector('#pairing').open);
  check(await paired.evaluate(()=>sessionStorage.getItem('pb.webuiToken')==='42'.repeat(32)),'pairing establishes a strong session bearer');
  check((await paired.locator('#conn').textContent())==='Controller not connected','paired disconnected-controller state is useful');
  // Remove only the new page's startup token-clearing script for reload.
  await paired.close();
  const restored=await page.context().newPage();
  await restored.addInitScript(()=>sessionStorage.setItem('pb.webuiToken','42'.repeat(32)));
  await restored.goto('http://127.0.0.1:8765/');await restored.waitForFunction(()=>saved!=='');
  await restored.reload();await restored.waitForFunction(()=>saved!=='');
  check(await restored.locator('#pairing[open]').count()===0,'browser reload restores the existing browser credential');
  await restored.evaluate(()=>{cur.A='CIRCLE';render()});
  let rejected=false;
  await restored.route('**/api/status',async route=>{if(!rejected){rejected=true;await route.fulfill({status:401,body:'rejected'})}else await route.continue()});
  await restored.locator('#pairing[open]').waitFor();
  check(await restored.evaluate(()=>sessionStorage.getItem('pb.webuiToken')===null&&cur.A==='CIRCLE'&&isDirty()),'rejected credential clears authentication and preserves the unsaved profile');
  await restored.locator('#pairing details summary').click();
  await restored.locator('#recoveryToken').fill('1'.repeat(64));await restored.locator('#tokenForm button').click();
  await restored.waitForFunction(()=>document.querySelector('#pairMessage').textContent.includes('rejected'));
  check(await restored.locator('#pairing[open]').count()===1,'invalid advanced token cannot dismiss pairing');
  await restored.locator('#recoveryToken').fill('42'.repeat(32));await restored.locator('#tokenForm button').click();
  await restored.waitForFunction(()=>!document.querySelector('#pairing').open);
  check(await restored.evaluate(()=>cur.A==='CIRCLE'&&isDirty()),'advanced recovery preserves the unsaved draft');
  await restored.unroute('**/api/status');
  await restored.route('**/api/status',route=>route.abort());
  await restored.waitForFunction(()=>document.querySelector('#conn').textContent==='Backend offline');
  check(await restored.locator('#pairing[open]').count()===0,'backend-offline state does not erase valid credentials');
  await restored.unroute('**/api/status');
  await restored.waitForFunction(()=>document.querySelector('#conn').textContent==='Controller not connected');
  check(await restored.evaluate(()=>portalTokenCache==='42'.repeat(32)),'backend reconnect reuses authenticated session');
  await restored.evaluate(()=>{cur=parse(saved);render()});await restored.close();
  // Game metadata must remain literal text in bridge diagnostics.
  await page.route('**/api/bridge',route=>route.fulfill({status:200,body:JSON.stringify({title:'PPSA00001',state:3,result:2,scan:0})}));
  const safeBridge=await page.evaluate(async()=>{
    games=[{id:'PPSA00001',name:'<img data-audit-title="injected">'}];lastBridge='';await paintBridge();
    return !document.querySelector('[data-audit-title]')&&document.querySelector('#bridgeOut').textContent.includes('<img data-audit-title="injected">');
  });
  check(safeBridge,'bridge game titles cannot create HTML elements');await page.unroute('**/api/bridge');
  // Icons use the same authenticated fetch path, then only local blob URLs.
  let iconBearer=false;
  await page.route('**/api/icon?*',async route=>{iconBearer=!!route.request().headers().authorization;await route.continue()});
  await page.evaluate(async()=>{clearIconUrls();await refreshGames();switchTab("games")});
  await page.waitForFunction(()=>Array.from(document.querySelectorAll('#games img')).some(img=>img.src.startsWith('blob:')&&img.naturalWidth===1));
  check(iconBearer,'game icon request carries bearer authentication');
  check(await page.evaluate(()=>!Array.from(document.querySelectorAll('img')).some(img=>img.getAttribute('src')?.startsWith('/api/icon'))),'protected icon URLs are never assigned directly to native images');
  await page.unroute('**/api/icon?*');await page.evaluate(()=>switchTab('remap'));
  // Only a corrupt server Default offers explicit recovery, with auth and confirmation.
  await page.evaluate(()=>{cur=parse(saved);render()});
  let recovered=false,recoveryAuthenticated=false;
  await page.route('**/api/profile?id=default',async route=>{
    if(route.request().method()==='GET'&&!recovered)await route.fulfill({status:500,body:'corrupt stored profile'});
    else await route.continue();
  });
  await page.route('**/api/recover-default',async route=>{recoveryAuthenticated=!!route.request().headers().authorization;recovered=true;await route.fulfill({status:200,body:'OK'})});
  await page.evaluate(()=>load('default',true));
  check(await page.locator('#profileRecovery').isVisible(),'corrupt Default presents an explicit recovery control');
  await page.evaluate(()=>window.confirm=()=>false);await page.locator('#recoverDefault').click();
  check(!recovered,'canceling corrupt Default recovery makes no mutation');
  await page.evaluate(()=>window.confirm=()=>true);await page.locator('#recoverDefault').click();
  await page.waitForFunction(()=>document.querySelector('#profileRecovery').hidden);
  check(recoveryAuthenticated,'Default recovery uses the authenticated mutation API');
  check(await page.evaluate(()=>editing==='default'&&!isDirty()),'Default recovery confirms canonical saved readback');
  await page.unroute('**/api/profile?id=default');await page.unroute('**/api/recover-default');
  check(errors.length===0, 'no JavaScript runtime errors');
  return {result:'PASS',checks:passed.length,passed,decoded,hardware:'NOT_RUN; loopback fixture only'};
}

/* Playwright CLI function, no product dependency. Requires webui_preview.py.
   Run: playwright-cli run-code --filename tests/webui_remap_browser.js
   The only API here is a loopback RAM fixture; this is not PS5 runtime proof. */
async (page) => {
  page = await page.context().newPage(); // Fresh document; never discard a user's open page.
  const passed = [];
  const check = (ok, name) => { if (!ok) throw new Error(name); passed.push(name); };
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
  const base = await page.evaluate(() => 'name=Default\n' + Object.entries(DEFAULTS).map(([k,v])=>k+'='+v).join('\n') + '\nSHIFT=L4\n');
  await page.request.post('http://127.0.0.1:8765/api/profile?id=default', {headers:{Authorization:'Bearer preview-only'},data:base});
  await page.reload();await page.waitForFunction(() => saved !== '' && cur.A === 'CROSS');
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
  check(outputGroups.join('|')==='Face buttons|D-pad|Shoulders & triggers|Sticks|System|Touchpad|Puckbridge actions','output actions grouped by function');
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
    check(!layout.overflow, 'no horizontal overflow at ' + width);
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
  check(errors.length===0, 'no JavaScript runtime errors');
  return {result:'PASS',checks:passed.length,passed,decoded,hardware:'NOT_RUN; loopback fixture only'};
}

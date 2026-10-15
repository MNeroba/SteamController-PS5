#include "vda_result.h"
#include "slot_registry.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks;assert(x); } while(0)
int main(void) {
    int32_t words[6]={0};
    const int32_t codes[]={7,0,-1,(int32_t)UINT32_C(0x803b0006),(int32_t)UINT32_C(0x803b0003)};
    const vda_result_kind_t kinds[]={VDA_RESULT_OWNED_RETURN,VDA_RESULT_NO_RECEIPT,VDA_RESULT_ERROR,VDA_RESULT_ASSIGNMENT_PENDING,VDA_RESULT_ERROR};
    for(unsigned i=0;i<5;i++) {
        vda_result_t r=vda_result_classify(codes[i],words);CHECK(r.kind==kinds[i]);CHECK(r.changed_words==0);
    }
    words[0]=7;words[5]=11;
    vda_result_t pending=vda_result_classify((int32_t)UINT32_C(0x803b0006),words);
    CHECK(pending.kind==VDA_RESULT_ASSIGNMENT_PENDING);CHECK(pending.changed_words==33);
    CHECK(strstr(vda_result_name(pending.kind),"ownership unresolved"));
    CHECK(vda_result_classify(0,words).kind==VDA_RESULT_NO_RECEIPT);
    CHECK(vda_result_classify(-1,NULL).kind==VDA_RESULT_ERROR);
    slot_registry_t registry;slot_registry_token_t token;
    CHECK(slot_registry_init(&registry)==0);CHECK(slot_registry_claim(&registry,"fixture",0x28de,0x1304,&token)==0);
    CHECK(slot_registry_activate(&registry,token)==0);
    unsigned creates=0;
    for(int i=0;i<32;i++) {
        if(slot_registry_begin_virtual_create(&registry,token)==0) {
            ++creates;
            CHECK(vda_result_classify((int32_t)UINT32_C(0x803b0006),words).kind==VDA_RESULT_ASSIGNMENT_PENDING);
            /* An output word or pending status never becomes a deletable ID. */
            CHECK(slot_registry_finish_virtual_create(&registry,token,-1)!=0);
        }
    }
    CHECK(creates==1);slot_registry_entry_t e;CHECK(slot_registry_snapshot(&registry,token,&e)==0);
    CHECK(e.virtual_blocked && !e.virtual_ready && e.virtual_handle<0);
    slot_registry_destroy(&registry);printf("VDA result/containment: %u checks passed\n",checks);return 0;
}

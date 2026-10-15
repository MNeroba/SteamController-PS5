/* SPDX-License-Identifier: GPL-3.0-or-later */
#pragma once
#include <stdint.h>

/* Backend result observation, not an alternative slot/ownership registry.
 * The six words retain their original zero input bytes. No speculative ABI
 * sentinel, klog ID, native-pad lookup or output word is promoted to a handle. */
typedef enum {
    VDA_RESULT_OWNED_RETURN, VDA_RESULT_ASSIGNMENT_PENDING,
    VDA_RESULT_NO_RECEIPT, VDA_RESULT_ERROR
} vda_result_kind_t;
typedef struct {
    vda_result_kind_t kind;
    uint32_t changed_words;
} vda_result_t;
static inline vda_result_t vda_result_classify(int32_t result,const int32_t words[6]) {
    vda_result_t out={VDA_RESULT_ERROR,0};
    if (result>0) out.kind=VDA_RESULT_OWNED_RETURN;
    else if ((uint32_t)result==UINT32_C(0x803b0006)) out.kind=VDA_RESULT_ASSIGNMENT_PENDING;
    else if (result==0) out.kind=VDA_RESULT_NO_RECEIPT;
    if (words) for(unsigned i=0;i<6;i++) if(words[i]!=0)out.changed_words|=1u<<i;
    return out;
}
static inline const char *vda_result_name(vda_result_kind_t kind) {
    switch(kind) {
        case VDA_RESULT_OWNED_RETURN:return "positive return receipt";
        case VDA_RESULT_ASSIGNMENT_PENDING:return "assignment pending; ownership unresolved";
        case VDA_RESULT_NO_RECEIPT:return "no ownership receipt";
        default:return "creation error; side effects unknown";
    }
}

#ifndef _LEECH_INSTANCE_H
#define _LEECH_INSTANCE_H

typedef struct LCH_Instance LCH_Instance;
typedef struct LCH_InstanceCreateInfo {
    char *instanceID;
    char *workDir;
} LCH_InstanceCreateInfo;

LCH_Instance *LCH_InstanceCreate(
    const LCH_InstanceCreateInfo *const createInfo);
void LCH_InstanceDestroy(LCH_Instance *instance);

#endif  // _LEECH_INSTANCE_H

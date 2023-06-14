#include "WateringManager.h"
#include "WateringManagerWrapper.h"

#ifdef __cplusplus
extern "C"
{
#endif
    struct tagWateringManager
    {
        WateringManager wateringManager;
    };
    struct tagWateringManager *GetInstance(void)
    {
        return new struct tagWateringManager;
    }
    void ReleaseInstance(struct tagWateringManager **ppInstance)
    {
        delete *ppInstance;
        *ppInstance = 0;
    }

#ifdef __cplusplus
};
#endif
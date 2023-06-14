#ifndef WATERING_MANAGER_WRAPPER_H
#define WATERING_MANAGER_WRAPPER_H
struct tagWateringManager;
#ifdef __cplusplus
extern "C"
{
#endif
    struct tagWateringManager *GetInstance(void);
    void ReleaseInstance(struct tagWateringManager **ppInstance);
#ifdef __cplusplus
};
#endif
#endif
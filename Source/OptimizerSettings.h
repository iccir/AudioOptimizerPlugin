#ifndef __OPTIMIZER_SETTINGS_H__
#define __OPTIMIZER_SETTINGS_H__

typedef struct {
    double frequency;
    double gain;
    double Q;
} IccirAudioOptimizerFilterParam;


const IccirAudioOptimizerFilterParam IccirAudioOptimizerSettingsFilterParam[] = {
    { 62.9, -11.4, 10.22 },
    {  115,  12.1,  4.23 },
    {  133, -22.7,  2.35 },
    {  264, -10.3,  1.00 },
    {  688,  -7.7,  5.00 }
};

// 1.0 = Apply filter as specified
double IccirAudioOptimizerGainMultiplier = 0.5;

#endif /* __OPTIMIZER_SETTINGS_H__ */

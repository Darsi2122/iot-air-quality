// Auto-generated decision tree. Do not edit by hand.
// Features (in order): CO, NO2, SO2, O3
// Classes: 0=Good, 1=Moderate, 2=Poor
#pragma once

static inline int aq_predict(float co, float no2, float so2, float o3) {
    if (co <= 435.5f) {
        if (co <= 204.5f) {
            if (so2 <= 5.55000019f) {
                if (co <= 183.5f) {
                    if (so2 <= 0.449999988f) {
                        return 0;   // Good
                    } else {
                        return 0;   // Good
                    }
                } else {
                    if (o3 <= 40.5f) {
                        return 0;   // Good
                    } else {
                        return 0;   // Good
                    }
                }
            } else {
                if (so2 <= 13.9499998f) {
                    if (o3 <= 86.5f) {
                        return 0;   // Good
                    } else {
                        return 1;   // Moderate
                    }
                } else {
                    if (no2 <= 19.4500008f) {
                        return 1;   // Moderate
                    } else {
                        return 1;   // Moderate
                    }
                }
            }
        } else {
            if (so2 <= 8.14999962f) {
                if (o3 <= 31.5f) {
                    if (co <= 255.5f) {
                        return 1;   // Moderate
                    } else {
                        return 1;   // Moderate
                    }
                } else {
                    if (o3 <= 78.5f) {
                        return 0;   // Good
                    } else {
                        return 1;   // Moderate
                    }
                }
            } else {
                if (o3 <= 86.5f) {
                    if (so2 <= 13.3500004f) {
                        return 1;   // Moderate
                    } else {
                        return 1;   // Moderate
                    }
                } else {
                    if (co <= 325.5f) {
                        return 1;   // Moderate
                    } else {
                        return 2;   // Poor
                    }
                }
            }
        }
    } else {
        if (so2 <= 18.25f) {
            if (o3 <= 34.5f) {
                if (no2 <= 30.1500015f) {
                    if (so2 <= 11.2999992f) {
                        return 1;   // Moderate
                    } else {
                        return 2;   // Poor
                    }
                } else {
                    if (so2 <= 13.3500004f) {
                        return 1;   // Moderate
                    } else {
                        return 1;   // Moderate
                    }
                }
            } else {
                if (so2 <= 10.9499998f) {
                    if (so2 <= 6.44999981f) {
                        return 0;   // Good
                    } else {
                        return 1;   // Moderate
                    }
                } else {
                    if (co <= 565.5f) {
                        return 2;   // Poor
                    } else {
                        return 2;   // Poor
                    }
                }
            }
        } else {
            if (co <= 563.5f) {
                if (o3 <= 66.5f) {
                    if (so2 <= 25.6500015f) {
                        return 1;   // Moderate
                    } else {
                        return 2;   // Poor
                    }
                } else {
                    if (o3 <= 149.5f) {
                        return 2;   // Poor
                    } else {
                        return 2;   // Poor
                    }
                }
            } else {
                if (so2 <= 29.6500015f) {
                    if (co <= 621.5f) {
                        return 2;   // Poor
                    } else {
                        return 2;   // Poor
                    }
                } else {
                    if (co <= 672.5f) {
                        return 2;   // Poor
                    } else {
                        return 2;   // Poor
                    }
                }
            }
        }
    }
}
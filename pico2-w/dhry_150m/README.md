# Pico 2 W board

Cortex M33 150 MHz
## dhrystone

### Flash cached
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[550-5190]:     2.320 
Dhrystones per Second:  431034.469 
DMIPS/MHz:      1.635
```

### Flash uncached
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[8753-143633]:  67.440
Dhrystones per Second:  14827.995
DMIPS/MHz:      0.056
```

/*
 * structs.h
 *
 *  Created on: Mar 19, 2022
 *      Author: wmhudson
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

typedef struct Scan {
    double distance;
    double angle;
} Scan;

typedef struct TallObject {
    int obj_num;
    int angle;
    float dist;
    int radial_width;
    float linear_width;
} TallObject;

#endif /* STRUCTS_H_ */

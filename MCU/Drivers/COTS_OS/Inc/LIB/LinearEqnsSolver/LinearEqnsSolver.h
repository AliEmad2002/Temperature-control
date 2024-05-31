/*
 * LinearEqnsSolver.h
 *
 *  Created on: May 31, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_LINEAREQNSSOLVER_LINEAREQNSSOLVER_H_
#define COTS_OS_INC_LIB_LINEAREQNSSOLVER_LINEAREQNSSOLVER_H_

/*
 * Solves a linear system of 'N' equations.
 *
 * Notes:
 * 		-	Let the system be:
 * 				4*x1	+ 3*x2	- x3	= -2,
 * 				-2*x1	- 4*x2	+ 5*x3	= 20,
 * 				x1		+ 2*x2	+ 6*x3	= 7,
 *
 * 			Then:
 * 				"pfAugArr" = {	4.0,	3.0,	-1.0,	-2.0,
 * 								-2.0,	-4.0,	5.0,	20.0,
 * 								1.0,	2.0,	6.0,	7.0		}
 *
 * 				"ucN" = 3
 *
 * 				"pfSol" must be initially allocated, after function return,
 * 				this array will be modified to the solution, i.e.: {x1, x2, x3}.
 * 				In this example, it will be modified to {3.0, -4.0, 2.0}
 *
 * 		-	If the system has no solution or if it is of an infinite number of
 * 			solutions, function returns 0. Otherwise, if system is successfully
 * 			solved, function returns 1.
 *
 */
uint8_t ucLIB_LinearEqnsSolver_Solve(float* pfAugArr, uint8_t ucN, float* pfSol);


#endif /* COTS_OS_INC_LIB_LINEAREQNSSOLVER_LINEAREQNSSOLVER_H_ */

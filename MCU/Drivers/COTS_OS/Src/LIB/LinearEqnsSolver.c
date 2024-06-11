/*
 * LinearEqnsSolver.c
 *
 *  Created on: May 31, 2024
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "stdlib.h"
#include "math.h"

/*	SELF	*/
#include "LIB/LinearEqnsSolver/LinearEqnsSolver.h"


/*
 * Note, TODO: This code is not the very efficient, used now to proof the concept of
 * black box plant modeling. optimize later.
 */
uint8_t ucLIB_LinearEqnsSolver_Solve(float* pfAugArr, uint8_t ucN, float* pfSol)
{
    for (uint8_t i = 0; i < ucN; i++)
    {
        // Partial pivoting for numerical stability
        uint8_t max_row = i;
        for (uint8_t k = i + 1; k < ucN; k++)
        {
            if (fabs(pfAugArr[k * (ucN + 1) + i]) > fabs(pfAugArr[max_row * (ucN + 1) + i]))
                max_row = k;
        }

        // Swap rows
        if (max_row != i)
        {
            for (uint8_t j = 0; j <= ucN; j++)
            {
                float temp = pfAugArr[i * (ucN + 1) + j];

                pfAugArr[i * (ucN + 1) + j] = pfAugArr[max_row * (ucN + 1) + j];

                pfAugArr[max_row * (ucN + 1) + j] = temp;
            }
        }

        // Perform forward elimination
        for (uint8_t k = i + 1; k < ucN; k++)
        {
            float factor = pfAugArr[k * (ucN + 1) + i] / pfAugArr[i * (ucN + 1) + i];

            for (uint8_t j = i; j <= ucN; j++)
                pfAugArr[k * (ucN + 1) + j] -= factor * pfAugArr[i * (ucN + 1) + j];
        }
    }

    // Back substitution
    for (int8_t i = ucN - 1; i >= 0; i--)
    {
        pfSol[i] = pfAugArr[i * (ucN + 1) + ucN];

        for (int8_t j = i + 1; j < ucN; j++)
            pfSol[i] -= pfAugArr[i * (ucN + 1) + j] * pfSol[j];

        pfSol[i] /= pfAugArr[i * (ucN + 1) + i];

        // Check for NaN solution:
        if (pfSol[i] == NAN)
        	return 0;

    }

    return 1;
}

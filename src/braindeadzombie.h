#ifndef BRAINDEADZOMBIE_H
#define BRAINDEADZOMBIE_H

#include "defines.h"
#include <math.h>
#include <stdio.h>

// Struct to cache (store) the danger level of a row.
// We cache this so we don't do heavy math 60 times a second.
typedef struct {
    int n;              // Number of plants in the row
    float S;            // Sum of plant Health Ratios
    float Q;            // Sum of Squares of Health Ratios
    bool mowerActive;   // Is the Lawn Mower still there?
    float cachedWeight; // The final Calculated Danger Score
} RowState;

// Global array to store the state of all 5 rows.
static RowState rowStates[GRID_ROWS];
static bool isInitialized = false;

// Function: Calculates n, S, and Q for a specific row.
    void CalculateRawStats(Cell grid[GRID_ROWS][GRID_COLS], Mower mowers[GRID_ROWS], int r, int *n, float *S, float *Q, bool *mower) {
    *n = 0;
    *S = 0.0f;
    *Q = 0.0f;
    *mower = (mowers[r].active && !mowers[r].triggered); // True if mower exists

    // Loop through columns starting from 1 (Column 0 is reserved/safe zone)
    for (int c = 1; c < GRID_COLS; c++) {
        PlantType type = grid[r][c].plantType;

        // EXCLUSION RULE:
        // Rose and Sunflower do not count towards the danger formula.
        if (type != PLANT_NONE && type != PLANT_SUNFLOWER && type != PLANT_ROSE) {
            (*n)++; // Increment plant count

            float maxHp = 100.0f;
            if (grid[r][c].maxHp > 0) maxHp = grid[r][c].maxHp;

            // Ratio = Current HP / Max HP (Percentage of life left)
            float ratio = (float)grid[r][c].hp / maxHp;
            if (ratio < 0) ratio = 0;

            *S += ratio;           // Add ratio to Sum
            *Q += (ratio * ratio); // Add ratio squared to Q
        }
    }
}

// Function: Updates the stats for ALL rows.
// Called once per frame in the update loop to keep AI data fresh.
void UpdateAllRowStats(Cell grid[GRID_ROWS][GRID_COLS], Mower mowers[GRID_ROWS]) {
    // Initialize cache on first run
    if (!isInitialized) {
        for(int i=0; i<GRID_ROWS; i++) rowStates[i].cachedWeight = 0;
        isInitialized = true;
    }

    for (int r = 0; r < GRID_ROWS; r++) {
        int current_n;
        float current_S, current_Q;
        bool current_mower;

        // Calculate current raw stats
        CalculateRawStats(grid, mowers, r, &current_n, &current_S, &current_Q, &current_mower);

        // OPTIMIZATION CHECK:
        // Only re-calculate the "Weight" (Determinant) if stats changed.
        // We use a small epsilon (0.001f) for float comparison.
        if (rowStates[r].n != current_n ||
            fabs(rowStates[r].S - current_S) > 0.001f ||
            fabs(rowStates[r].Q - current_Q) > 0.001f ||
            rowStates[r].mowerActive != current_mower) {

            // Update Cache
            rowStates[r].n = current_n;
            rowStates[r].S = current_S;
            rowStates[r].Q = current_Q;
            rowStates[r].mowerActive = current_mower;

            // SCALAR DETERMINANT FORMULA:
            // This represents the "Defense Strength" of the row.
            // Formula: 1 + n^2 + S^2 + Q^2
            float det = 1.0f + (current_n * current_n) + (current_S * current_S) + (current_Q * current_Q);

            // TOTAL WEIGHT:
            // 10 points if Mower exists + Determinant.
            // Higher Weight = More Dangerous.
            // Lower Weight = Easier for Zombie.
            rowStates[r].cachedWeight = (10.0f * (current_mower ? 1.0f : 0.0f)) + det;
        }
    }
}

// Function: Finds the row with the LOWEST weight (Easiest path).
int PickBestStartRow() {
    float minWeight = 999999.0f;
    int bestRow = 0;

    for (int r = 0; r < GRID_ROWS; r++) {
        if (rowStates[r].cachedWeight < minWeight) {
            minWeight = rowStates[r].cachedWeight;
            bestRow = r;
        }
    }
    return bestRow;
}

#endif
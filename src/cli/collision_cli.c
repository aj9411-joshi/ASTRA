/* ASTRA collision detection - console version. */
#include <stdio.h>
#include <math.h>
#include "collision.h"

#define MAX_OBJECTS 50

int main(void) {
    char   names[MAX_OBJECTS][30];
    double x[MAX_OBJECTS], y[MAX_OBJECTS];
    int n, alerts = 0, criticalCount = 0, warningCount = 0;
    double minDistance = 0.0;
    int closest1 = -1, closest2 = -1;

    printf("====================================\n");
    printf(" ASTRA COLLISION DETECTION SYSTEM\n");
    printf("====================================\n");
    printf("Enter number of space objects (1-%d): ", MAX_OBJECTS);

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_OBJECTS) {
        printf("Invalid number of objects!\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("\nEnter name of object %d: ", i + 1);
        if (scanf("%29s", names[i]) != 1) { printf("Invalid object name!\n"); return 1; }
        printf("Enter X coordinate: ");
        if (scanf("%lf", &x[i]) != 1 || !isfinite(x[i])) { printf("Invalid X coordinate!\n"); return 1; }
        printf("Enter Y coordinate: ");
        if (scanf("%lf", &y[i]) != 1 || !isfinite(y[i])) { printf("Invalid Y coordinate!\n"); return 1; }
    }

    printf("\n====================================\n");
    printf("       COLLISION ALERT REPORT\n");
    printf("====================================\n");

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double d = calculateDistance(x[i], y[i], x[j], y[j]);

            if (closest1 == -1 || d < minDistance) {
                minDistance = d;
                closest1 = i;
                closest2 = j;
            }
            if (d <= ASTRA_WARNING_DISTANCE) {
                alerts++;
                printf("\nObject 1: %s\nObject 2: %s\n", names[i], names[j]);
                printf("Distance: %.2f\nRisk Level: %s\n", d, getRiskLevel(d));
                if (d <= ASTRA_CRITICAL_DISTANCE) criticalCount++; else warningCount++;
                printf("------------------------------------\n");
            }
        }
    }

    printf("\n====================================\n");
    printf("          ASTRA ALERT SUMMARY\n");
    printf("====================================\n");
    printf("Total Alerts: %d\nCritical Alerts: %d\nWarning Alerts: %d\n",
           alerts, criticalCount, warningCount);
    if (alerts == 0) printf("No proximity alerts detected.\n");

    if (closest1 != -1) {
        printf("\n====================================\n");
        printf("        MOST DANGEROUS PAIR\n");
        printf("====================================\n");
        printf("Object 1: %s\nObject 2: %s\n", names[closest1], names[closest2]);
        printf("Minimum Distance: %.2f\nRisk Level: %s\n", minDistance, getRiskLevel(minDistance));
    } else {
        printf("\nAt least two objects are required to find a pair.\n");
    }

    printf("\nASTRA analysis completed.\n");
    return 0;
}

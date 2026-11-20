#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pigpio.h>

#define PIN 17
#define FILENAME "../data/temperature_log.txt"

/* Set warning LED state using pigpio on GPIO 17 */
void setWarningLed(int state) {
    gpioWrite(PIN, state ? 1 : 0);
}

/* Returns 1 if temperature is outside [min_temp, max_temp], 0 if within range */
int isOutOfRange(double temperature, double min_temp, double max_temp) {
    return (temperature < min_temp || temperature > max_temp) ? 1 : 0;
}

/* Calculates average of recorded temperatures */
double calculateAverage(const double readings[], int count) {
    double total = 0.0;
    for (int i = 0; i < count; i++) {
        total += readings[i];
    }
    return (count > 0) ? (total / count) : 0.0;
}


int main(int argc, char *argv[]) {

    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize pigpio.\n");
        return 1;
    }
    gpioSetMode(PIN, PI_OUTPUT);
    setWarningLed(0);

    int choice;
    double min_temp = 68.0;
    double max_temp = 76.0;

    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening %s for writing.\n", FILENAME);
        setWarningLed(0);
        gpioTerminate();
        return 1;
    }
    fprintf(file, "Temperature\tSystem Status\tWarning LED\n");
    fclose(file);

    printf("========================================\n");
    printf("SMART CLASSROOM COMFORT MONITOR\n");
    printf("========================================\n");
    printf("Acceptable Range: %.1f°F - %.1f°F (inclusive)\n", min_temp, max_temp);

    while (1) {
        printf("\n--- Menu ---\n");
        printf("1. Enter classroom temperature readings\n");
        printf("2. Change acceptable temperature range\n");
        printf("3. Exit program\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 1) {
            int num_inputs;
            printf("How many temperature readings would you like to enter? ");
            if (scanf("%d", &num_inputs) != 1 || num_inputs <= 0) {
                printf("Invalid number of inputs.\n");
                while (getchar() != '\n');
                continue;
            }

            double *readings = malloc((size_t)num_inputs * sizeof(double));
            if (readings == NULL) {
                printf("Memory allocation failed.\n");
                continue;
            }

            for (int i = 0; i < num_inputs; i++) {
                printf("Enter temperature reading %d of %d (°F): ", i + 1, num_inputs);
                if (scanf("%lf", &readings[i]) != 1) {
                    printf("Invalid value. Please re-enter.\n");
                    while (getchar() != '\n');
                    i--;
                    continue;FILENAME
                }

                int warning = isOutOfRange(readings[i], min_temp, max_temp);
                const char *status = warning ? "WARNING" : "NORMAL";
                const char *led_text = warning ? "ON" : "OFF";

                setWarningLed(warning);

                printf("\nTemperature System Status Warning LED\n");
                printf("%.0f°F %s %s\n\n", readings[i], status, led_text);

                file = fopen(FILENAME, "a");
                if (file != NULL) {
                    fprintf(file, "%.1f°F\t%s\t%s\n", readings[i], status, led_text);
                    fclose(file);
                }
            }
            
            printf("Average temperature: %.2f°F\n", calculateAverage(readings, num_inputs));
            free(readings);

        } else if (choice == 2) {
            double new_min, new_max;
            printf("Enter new minimum temperature (°F): ");
            if (scanf("%lf", &new_min) != 1) {
                printf("Invalid input.\n");
                while (getchar() != '\n');
                continue;
            }
            printf("Enter new maximum temperature (°F): ");
            if (scanf("%lf", &new_max) != 1 || new_max < new_min) {
                printf("Invalid maximum temperature (must be >= minimum).\n");
                while (getchar() != '\n');
                continue;
            }
            min_temp = new_min;
            max_temp = new_max;
            printf("Acceptable range updated to: %.1f°F - %.1f°F\n", min_temp, max_temp);

        } else if (choice == 3) {
            printf("\nExiting program and turning Warning LED OFF...\n");
            setWarningLed(0);
            break;

        } else {
            printf("Invalid choice. Please select 1, 2, 3, or 4.\n");
        }
    }

    gpioTerminate();
    return 0;
}

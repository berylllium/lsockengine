/**
 * @file clock.h
 * @brief This header file contains the clock structure and all its corresponding functions.
 */
#pragma once

/**
 * @brief The clock data.
 * 
 * A clock can be used in many ways; to store a certain moment in time, to calculate the elapsed time since a moment in
 * time, etc.
 */
typedef struct lise_clock
{
	/**
	 * @brief A point in time in seconds.
	 */
	double start_time;
} lise_clock;

/**
 * @brief Resets the clock, setting the \ref start_time to the current time.
 * 
 * The stored time is arbitrary, meaning that it has no meaning on its own. This time can only be meaningfully used
 * when calculating differences in time.
 * 
 * @param clock The clock to reset.
 */
void lise_clock_reset(lise_clock* clock);

/**
 * @brief Calculates the elapsed time in seconds since the stored \ref start_time and the current time.
 * 
 * @param clock The clock to calculate the elapsed time from.
 * @return double The elapsed time in seconds.
 */
double lise_clock_get_elapsed_time(lise_clock clock);

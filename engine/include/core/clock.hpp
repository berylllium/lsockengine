/**
 * @file clock.hpp
 * @brief This header file contains the clock structure and all its corresponding functions.
 */
#pragma once

namespace lise
{

/**
 * @brief The clock data.
 * 
 * A clock can be used in many ways; to store a certain moment in time, to calculate the elapsed time since a moment in
 * time, etc.
 */
struct Clock
{
	/**
	 * @brief A point in time in seconds.
	 */
	double start_time;

	/**
	 * @brief Resets the clock, setting the \ref start_time to the current time.
	 * 
	 * The stored time is arbitrary, meaning that it has no meaning on its own. This time can only be meaningfully used
	 * when calculating differences in time.
	 * 
	 * @param clock The clock to reset.
	 */
	void reset();

	/**
	 * @brief Calculates the elapsed time in seconds since the stored \ref start_time and the current time.
	 * 
	 * @param clock The clock to calculate the elapsed time from.
	 * @return double The elapsed time in seconds.
	 */
	double get_elapsed_time() const;
};

}

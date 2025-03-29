#include "itcMutex.h"

namespace ItcPlatform
{
/***
 * Please do not use anything in this namespace outside itc-platform project,
 * since it's for private usage
 */
namespace INTERNAL
{

unsigned long int getTimeDiffInNanoSec(struct timespec t_start, struct timespec t_end)
{
    unsigned long int diff = 0;
	if(t_end.tv_nsec < t_start.tv_nsec)
	{
		diff = (t_end.tv_sec - t_start.tv_sec)*1000000000 - (t_start.tv_nsec - t_end.tv_nsec);
	} else
	{
		diff = (t_end.tv_sec - t_start.tv_sec)*1000000000 + (t_end.tv_nsec - t_start.tv_nsec);
	}
	return diff;
}

} // namespace INTERNAL
} // namespace ItcPlatform
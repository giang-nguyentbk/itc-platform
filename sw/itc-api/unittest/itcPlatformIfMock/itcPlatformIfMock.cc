#include "itcPlatformIfMock.h"
#include "itcConstant.h"

namespace ITC
{
namespace PROVIDED
{
std::weak_ptr<ItcPlatformIf> ItcPlatformIf::getInstance()
{
	return std::static_pointer_cast<ItcPlatformIf>(INTERNAL::ItcPlatformIfMock::getInstance().lock());
}
}

namespace INTERNAL
{
SINGLETON_DEFINITION(ItcPlatformIfMock)

} // namespace INTERNAL
} // namespace ITC


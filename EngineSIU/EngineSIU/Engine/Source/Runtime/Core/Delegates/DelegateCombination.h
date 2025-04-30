#pragma once
#include "Delegate.h"


#define DECLARE_DELEGATE(DelegateName) FUNC_DECLARE_DELEGATE(DelegateName, void)
#define DECLARE_DYNAMIC_DELEGATE(DelegateName) FUNC_DECLARE_DELEGATE(DelegateName, void)
#define DECLARE_MULTICAST_DELEGATE(DelegateName) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE(DelegateName)  FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void)

#define DECLARE_DELEGATE_OneParam(DelegateName, Param1Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type)
#define DECLARE_DYNAMIC_DELEGATE_OneParam(DelegateName, Param1Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type)
#define DECLARE_MULTICAST_DELEGATE_OneParam(DelegateName, Param1Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(DelegateName, Param1Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type)

#define DECLARE_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type)
#define DECLARE_DYNAMIC_DELEGATE_TwoParams(DelegateName, P1, P2) FUNC_DECLARE_DELEGATE(DelegateName, void, P1, P2)
#define DECLARE_MULTICAST_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type, Param2Type)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(DelegateName, P1, P2) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, P1, P2)

#define DECLARE_DELEGATE_ThreeParams(DelegateName, Param1Type, Param2Type, Param3Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type)
#define DECLARE_DYNAMIC_DELEGATE_ThreeParams(DelegateName, P1, P2, P3) FUNC_DECLARE_DELEGATE(DelegateName, void, P1, P2, P3)
#define DECLARE_MULTICAST_DELEGATE_ThreeParams(DelegateName, Param1Type, Param2Type, Param3Type) FUNC_DECLARE_MULTICAST_DELEGATE( DelegateName, void, Param1Type, Param2Type, Param3Type)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(DelegateName, P1, P2, P3) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, P1, P2, P3)

#define DECLARE_DELEGATE_FourParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type)
#define DECLARE_DYNAMIC_DELEGATE_FourParams(DelegateName, P1, P2, P3, P4) FUNC_DECLARE_DELEGATE(DelegateName, void, P1, P2, P3, P4)
#define DECLARE_MULTICAST_DELEGATE_FourParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(DelegateName, P1, P2, P3, P4) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, P1, P2, P3, P4)

#define DECLARE_DELEGATE_FiveParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type)
#define DECLARE_DYNAMIC_DELEGATE_FiveParams(DelegateName, P1, P2, P3, P4, P5) FUNC_DECLARE_DELEGATE(DelegateName, void, P1, P2, P3, P4, P5)
#define DECLARE_MULTICAST_DELEGATE_FiveParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(DelegateName, P1, P2, P3, P4, P5) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, P1, P2, P3, P4, P5)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(DelegateName, P1, P2, P3, P4, P5, P6) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, P1, P2, P3, P4, P5, P6)

/**
 * Declares a multicast delegate that is meant to only be activated from OwningType
 * NOTE: This behavior is not enforced and this type should be considered deprecated for new delegates, use normal multicast instead
 */
#define DECLARE_EVENT( OwningType, EventName ) FUNC_DECLARE_EVENT( OwningType, EventName, void )

/**
 * Declares a multicast delegate that is meant to only be activated from OwningType
 *
 * @note: This behavior is not enforced and this type should be considered deprecated for new delegates, use normal multicast instead
 */
#define FUNC_DECLARE_EVENT( OwningType, EventName, ReturnType, ... ) \
class EventName : public TMulticastDelegate<ReturnType(__VA_ARGS__)> \
{ \
friend class OwningType; \
};

#define MAC 1
#define WINDOWS 0
#define SGI 0
#define MOTIF 0
#define BEBOX 0
#define RHAPSODY 0

#define TARGET_OS_MAC 1
#define OPAQUE_TOOLBOX_STRUCTS 0
#define ACCESSOR_CALLS_ARE_FUNCTIONS 0
#define CALL_NOT_IN_CARBON 1
#define USENAVSERVICES 1

#define GetPortBounds(port,rect)  *(rect) = port->portRect
#define GetPortPixMap(port) port->portPixMap

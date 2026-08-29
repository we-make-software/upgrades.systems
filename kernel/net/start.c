#include"net.h"
status net_start(void)
{
	net_discovery scan;
	return(net_scan(&scan));
}

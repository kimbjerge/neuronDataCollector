/*
 * NETServer.c
 *
 *  Created on: 27. nov. 2018
 *      Author: Kim Bjerge
 */
//DEFINE STATEMENTS TO INCREASE SPEED
//#undef LWIP_TCP
//#undef LWIP_DHCP
//#undef CHECKSUM_CHECK_UDP
//#undef LWIP_CHECKSUM_ON_COPY
//#undef CHECKSUM_GEN_UDP

// lwip board support packaged is optimized:
// See link to optimize speed: http://zedboard.org/content/zedboard-ethernet-udp-communication
// temac_adapter_options:
//     n_rx_desc = 256
//     n_tx_desc = 256
//     phy_link_speed = Autodetect
// lwip_memory_options:
//     memp_n_pbuf = 1024
//     memp_n_tcp_seg = 1024
// pbuf_options:
//     pbuf_pool_size = 1024
// tcp_options:
//     tcp_wnd = 4096

#include <stdio.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform_config.h"
#include "xil_printf.h"

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif

#include "lwip/udp.h"

// Include network definitions
#include "defsNet.h"

extern "C" {
 	// missing declaration in lwIP
	void lwip_init();
}

#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif

/* set up netif stuctures */
static struct netif server_netif;

void (*socket_thread)(void *) = 0;

void
print_ip(char *msg, struct ip_addr *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void
print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

	print_ip((char*)"Board IP: ", ip);
	print_ip((char*)"Netmask : ", mask);
	print_ip((char*)"Gateway : ", gw);
}

static void network_thread(void *p)
{
    struct netif *netif;
    struct ip_addr ipaddr, netmask, gw;
#if LWIP_DHCP==1
    int mscnt = 0;
#endif
    /* the mac address of the board. this should be unique per board */
    unsigned char mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

    netif = &server_netif;

#if LWIP_DHCP==0
    /* initliaze IP addresses to be used */
    IP4_ADDR(&ipaddr,  192, 168, 1, 10);
    IP4_ADDR(&netmask, 255, 255, 255,  0);
    IP4_ADDR(&gw,      192, 168, 1, 1);
#endif

    /* print out IP settings of the board */
    xil_printf("\r\n\r\n");
    xil_printf("----- lwIP Netork Server Starting ------\r\n");

#if LWIP_DHCP==0
	xil_printf("Board IP settings: \r\n");
    print_ip_settings(&ipaddr, &netmask, &gw);
    /* print all application headers */
#endif

#if LWIP_DHCP==1
	ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#endif
    /* Add network interface to the netif_list, and set it as default */
    if (!xemac_add(netif, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
        xil_printf("Error adding N/W interface\r\n");
        return;
    }
    netif_set_default(netif);

    /* specify that the network if is up */
    netif_set_up(netif);

    /* start packet receive thread - required for lwIP operation */
    sys_thread_new("xemacif_input_thread", (void(*)(void*))xemacif_input_thread, netif,
            THREAD_STACKSIZE,
            DEFAULT_THREAD_PRIO);

#if LWIP_DHCP==1
    dhcp_start(netif);
    while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
#else
    /* start packet receive thread - required for lwIP operation */
    sys_thread_new("socket_thread", (void(*)(void*))socket_thread, 0,
					THREAD_STACKSIZE,
					DEFAULT_THREAD_PRIO);
    vTaskDelete(NULL);
#endif
    return;
}

int main_net_thread()
{
#if LWIP_DHCP==1
	int mscnt = 0;
#endif

	/* initialize lwIP before calling network_thread */
    lwip_init();

    sys_thread_new("NW_THRD", network_thread, NULL,
    				THREAD_STACKSIZE,
					DEFAULT_THREAD_PRIO);

#if LWIP_DHCP==1
    while (1) {

    	vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		if (server_netif.ip_addr.addr) {
			xil_printf("DHCP request success\r\n");
			print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
		    /* start socket thread - required for lwIP operation */
		    sys_thread_new("socket_thrd", (void(*)(void*))socket_thread, 0,
							THREAD_STACKSIZE,
							DEFAULT_THREAD_PRIO);
			break;
		}
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= 10000) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			xil_printf("Configuring default IP of 192.168.1.10\r\n");
			IP4_ADDR(&(server_netif.ip_addr),  192, 168, 1, 10);
			IP4_ADDR(&(server_netif.netmask), 255, 255, 255,  0);
			IP4_ADDR(&(server_netif.gw),  192, 168, 1, 1);
			print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
		    /* start socket thread - required for lwIP operation */
		    sys_thread_new("socket_thrd", (void(*)(void*))socket_thread, 0,
							THREAD_STACKSIZE,
							DEFAULT_THREAD_PRIO);
			break;
		}
	}
#endif

    vTaskDelete(NULL);
    return 0;
}

void init_net_server(void (*net_thread)(void *))
{
	socket_thread = net_thread;
	if (socket_thread != 0)
		sys_thread_new("main_net_thrd", (void(*)(void*))main_net_thread, 0,
						THREAD_STACKSIZE,
						DEFAULT_THREAD_PRIO);
}


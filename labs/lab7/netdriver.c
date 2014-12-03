#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>


struct os_priv {
    struct net_device_stats stats;
    struct sk_buff *skb;
    struct os_packet *pkt;
    struct net_device *dev;
 };  
 
 struct net_device *os0, *os1;

 int os_open(struct net_device *dev) { return 0; }
  int os_stop(struct net_device *dev) { return 0; }
  int os_start_xmit(struct sk_buff *skb, struct net_device *dev) { return 0; }
  struct net_device_stats *os_stats(struct net_device *dev) {
     return &(((struct os_priv*)netdev_priv(dev))->stats);
  }
  static const struct net_device_ops os_device_ops = {
     .ndo_open = os_open,
     .ndo_stop = os_stop,
     .ndo_start_xmit = os_start_xmit,
     .ndo_get_stats = os_stats,
  };
  
  int os_header(struct sk_buff *skb, struct net_device *dev,
                unsigned short type, const void *daddr, const void *saddr,
                unsigned int len) {
     return 0;
  };
  
  static const struct header_ops os_header_ops = {
      .create  = os_header,
   };
int your_open(struct net_device *dev) {  /* start the network queue */  }

  int your_stop(struct net_device *dev) {  /* stop the network queue */  }

  static void your_tx_i_handler(struct net_device *dev) {
     /* normally stats are kept - you do not need to do much
        here if you do not want to except resume the queue 
        if it is not accepting packets.
      */
  }

  static void your_rx_i_handler(struct net_device *dev) {
     /* allocate space for a socket buffer
        add two bytes of space to align on 16 byte boundary
        copy the packet from the private part of dev to the socket buffer
        set the protocol field of the socket buffer using 'eth_type_trans'
        set the dev field of the socket buffer to dev (argument)
        invoke 'netif_rx()' on the socket buffer
        resume the network queue if it is not accepting packets
      */
  }

  netdev_tx_t your_start_xmit(struct sk_buff *skb, struct net_device *dev) {
     /* pull the packet and its length from skb
        locate the IP header in the packet (after the eth header below)
        switch the third octet of the source and destination addresses
        save the modified packet (even add some data if you like)
          in the private space reserved for it
        simulate a receive interrupt by calling 'your_rx_i_handler'
        simulate a transmit interrupt by calling 'your_tx_i_handler'
        free skb
      */
  }

  int your_header(struct sk_buff *skb, struct net_device *dev,
                 unsigned short type, const void *daddr, const void *saddr,
                 unsigned int len) {
     /* make room for the header in the socket buffer (from argument 'skb')
        set the protocol field (from argument 'type')
        copy the address given by the device to both source and
          destination fields (from argument 'dev')
        reverse the LSB on the destination address
      */
  }
  
  struct net_device_stats *your_stats(struct net_device *dev) {
      struct os_priv *priv = netdev_priv(dev);
      return &priv->stats;
  }
  
  

  static const struct header_ops your_header_ops = {
     .create  = your_header,
  };

  static const struct net_device_ops your_device_ops = {
     .ndo_open = your_open,
     .ndo_stop = your_stop,
     .ndo_start_xmit = your_start_xmit,
     .ndo_get_stats = your_stats,
  };

  int init_module (void) {
     /* allocate two ethernet devices
        set MAC addresses and broadcast values
        set device names
        set network device operations
        set network header creation operation
        set NOARP flags
        kmalloc space for a packet
        register both network devices
      */
      struct os_priv *priv0;
      struct os_priv *priv1;
	  int i;
	  
      os0 = alloc_etherdev(sizeof(struct os_priv));
      os1 = alloc_etherdev(sizeof(struct os_priv));
	  
	  for (i=0 ; i < 6 ; i++) os0->dev_addr[i] = (unsigned char)i;
	     for (i=0 ; i < 6 ; i++) os0->broadcast[i] = (unsigned char)15;
	     os0->hard_header_len = 14;
	 os1->dev_addr[5]++;
	 
	    memcpy(os0->name, "os0\0", 4);
	   memcpy(os1->name, "os1\0", 4);
	   
	   os0->netdev_ops = &os_device_ops;
	       os1->netdev_ops = &os_device_ops;
		   os0->header_ops = &os_header_ops;
		   os1->header_ops = &os_header_ops;
		   
		   os0->flags |= IFF_NOARP;
		     os1->flags |= IFF_NOARP;
			  if(dev == os0) {
				 priv0 = netdev_priv(os0);
				 priv0->pkt->length = len
				   memset(priv0, 0, sizeof(struct os_priv));
				   }
				 
  				 priv1 = netdev_priv(os1);
  				   memset(priv1, 0, sizeof(struct os_priv));
				   
				   // @@@DEB
			       priv0->dev = os0;
			       priv1->dev = os1;
			       if (register_netdev(os0) || register_netdev(os1))
			           printk(KERN_INFO "loopback: error registering os0 or os1\n");
			       else
			           printk(KERN_INFO "loopback: os0 and os1 registered\n");

			       printk(KERN_INFO "loopback: Module loaded successfully\n");
			       return 0;
  }

  void cleanup_module (void) {
     /* free the packet space
        unregister the network devices
      */
	  struct os_priv *priv;
	    if (os0) {
	       priv = netdev_priv(os0);
	       kfree(priv->pkt);
	       unregister_netdev(os0);
	    }
		 if (os1) {
	         priv = netdev_priv(os1);
	         kfree(priv->pkt);
	         unregister_netdev(os1);
	      }
  }
  
  

  MODULE_LICENSE("GPL");
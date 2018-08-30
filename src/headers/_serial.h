#include <libserialport.h> // cross platform serial port lib
#include <sysexits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*! \brief indentify port */
static struct sp_port *port;
/*! \brief available ports */
static struct sp_port **ports;


///Error enum 
enum serial_err {
	E_OK = 0,	     ///< No errors
	E_COM,		     ///< Serial port has`t opened
	E_OPEN = EX_NOINPUT, ///< File /dev/tty* did not exit
	E_DETECTION,	     ///< No serial devices
	E_UNPLUGGED,	     ///< Device has been unpugged or no connection
};


char *serial_errmsg[] = {
 [E_OK] =   "\n",
 [E_COM] =  "IdentifyErr: Error opening serial device.\n",
 [E_DETECTION] = "IdentifyErr: No serial devices detected.\n",
 [E_UNPLUGGED] = "IdentifyErr: Device has been unplugged."
};

/*!
 @brief database,which contains encrypted keys 
*/
struct ibutton_keys_{
const char *id;
char hash_str[65];
};


/*!
	@brief ERROR EXIT
*/
static inline void error_exit(enum serial_err err)
{
    fprintf(stderr, "%s\n", serial_errmsg[err]);
    exit(err);
}
/*! 
	@fn static inline enum serial_err open_read_com(const char *the_port,
					    unsigned int attempt,
					    unsigned int baudrate,
					    char *data,struct ibutton_keys_ hash[],
					    unsigned int cnt_keys,unsigned int size)
	@brief This function receives hashes via UART interface
	@param the_port tty COM Port
	@param baudrate UART baud rate 
	@param ttl time for ibutton connection in seconds
	@param data output hex string with hash
	@param hash[] a set of already calculated hashed 
	@param cnt_keys number of keys in config database
	@param size number of symbols for UART receive
*/
static inline enum serial_err open_read_com(const char *the_port,
					    unsigned int ttl,
 					    unsigned int baudrate,
  					    char *data,struct ibutton_keys_ hash[],
					    unsigned int cnt_keys,unsigned int size)
{
if(sp_get_port_by_name(the_port,&port) != SP_OK)
	error_exit(E_OPEN);

if(sp_open(port,SP_MODE_READ) != SP_OK) 
	error_exit(E_COM);

	sp_free_port_list(ports);
        sp_set_baudrate(port,baudrate);

        int bytes_waiting = 0,
	    cnt_attempts;		
	unsigned int tries = 0;
	while(1){
		bytes_waiting = sp_input_waiting(port);
		if (bytes_waiting > 0) {
			   int byte_num = 0;

			/* use only nonblocking type for runtime */
			   byte_num = sp_nonblocking_read(port,data,size);
			   if(!byte_num) 
			      sleep(0.5);
		}
		for(cnt_attempts = 0;cnt_attempts < cnt_keys;cnt_attempts++){
			/* search concurrences  with UART data and hashes */
			if(strncmp(hash[cnt_attempts].hash_str,data,64) == 0){
			  sp_close(port);
		  	  return E_OK;
			}			
		}
		 sleep(1);
	         tries++;
		if(tries > ttl){
		  sp_close(port);
		  return E_UNPLUGGED;
		}
		/* Device has been unplugged or any serial data */
		 if(bytes_waiting < 0){
		  sp_close(port);
		  error_exit(E_UNPLUGGED); 
		}
	}

	  sp_free_port_list(ports);
 	  sp_close(port);
}

/*!
	@brief search avaliable COM ports 
	@return name of available COM Port

*/
static inline char *list_ports(void) 
{
	int i;
	if (sp_list_ports(&ports) != SP_OK) 
	  error_exit(E_DETECTION);

	/* Search an available serial */
	for (i = 0; ports[i]; i++);

	/* dir to tty com device*/
	return sp_get_port_name(ports[i-1]);
}

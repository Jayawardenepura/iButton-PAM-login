#include <libserialport.h> // cross platform serial port lib
#include <sysexits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

///Error enum 
enum serial_err {
	E_OPEN = EX_NOINPUT, ///< File /dev/tty* did not exit
	E_OK = 0,	     ///< No errors
	E_COM,		     ///< Serial port has`t opened
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
const char *ibutton_crc_id;
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


/*! \brief indentify port */
static struct sp_port *serial_port;
/*! \brief available port */
static struct sp_port **detected_ports;


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
 					    long baudrate,
  					    char *data,struct ibutton_keys_ hash[],
					    unsigned int cnt_keys,unsigned int size)
{
if(sp_get_port_by_name(the_port,&serial_port) != SP_OK)
	error_exit(E_OPEN);

if(sp_open(serial_port,SP_MODE_READ) != SP_OK) 
	error_exit(E_COM);

	sp_free_port_list(detected_ports);
        sp_set_baudrate(serial_port,baudrate);
        int bytes_waiting;
	unsigned int timeout = 0;
	while(1){
		bytes_waiting = sp_input_waiting(serial_port);
		if (bytes_waiting > 0){
			   int byte_num = 0;
			/* use only nonblocking type for runtime */
			   byte_num = sp_nonblocking_read(serial_port,data,size);
			   if(!byte_num) 
			   	sleep(0.5);
		}
		for(int db_field = 0;db_field < cnt_keys;db_field++)
			/* search concurrences  with UART data and hashes */
			if(strncmp(hash[db_field].hash_str,data,64) == 0){
			  sp_close(serial_port);
		  	  return E_OK;
			}			
		sleep(1);
		timeout++;
		if(timeout > ttl){
			sp_close(serial_port);
			return E_UNPLUGGED;
		}
		/* Device has been unplugged or any serial data */
		if(bytes_waiting < 0){
			sp_close(serial_port);
			error_exit(E_UNPLUGGED); 
		}
	}
}

/*!
	@brief search avaliable COM ports 
	@return name of available COM Port
*/
static inline char *list_ports(void) 
{
	if (sp_list_ports(&detected_ports) != SP_OK) 
		error_exit(E_DETECTION);
	unsigned int aPort;
	/* Search an available serial port*/
	for (aPort = 0; detected_ports[aPort]; aPort++);
	/*return string dir to tty com device*/
	return sp_get_port_name(detected_ports[--aPort]);
}

#include <libserialport.h> // cross platform serial port lib
#include <libconfig.h>   
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "headers/_serial.h"
#include "headers/notify.h"
/*encryptions*/
#include "headers/sha256.h"
#include "headers/hmac-sha256.h"

#define PAM_CONFIG "/etc/ibutton_pam.config"

/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	return PAM_SUCCESS;
}
PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

/*! 
	@brief This is the base function for authorization.	

*/
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv) {

	config_t cfg;               /*Returns all parameters in this structure */
	config_setting_t *setting;

	const char *config_file_name = PAM_CONFIG;

	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if (!config_read_file(&cfg, config_file_name)){
		config_destroy(&cfg);
		return PAM_SERVICE_ERR;
	}

	const char *direct_to_dev; 
	static int _baudrate,attempts;
	/*Read the parameter group*/
	setting = config_lookup(&cfg, "pam_params");

	if(setting != NULL)
	{
	/* read the fields of the config */

	/* Read the integer value with time for auth */
	if(!config_setting_lookup_int(setting, "time_to_live", &attempts))
		return PAM_SERVICE_ERR;

	if(!config_setting_lookup_int(setting, "baudrate", &_baudrate))
		return PAM_SERVICE_ERR;

	/* Read the string with device dir */
	if(!config_setting_lookup_string(setting, "arduino_dev", &direct_to_dev))
		return PAM_SERVICE_ERR;
	}

	/*************************************************************************/

	bool searched_dev;
	/* you should create rules for tty avr devices, see 09-uno.rules*/
	searched_dev = search_com_device(direct_to_dev,attempts); /* PORT */
	if(false == searched_dev)
		return PAM_AUTH_ERR;	

	config_setting_t *hmackey;
	hmackey = config_lookup(&cfg, "hmac_key");
	if(hmackey == NULL)
		return PAM_SERVICE_ERR;

	const char *hmac_key_sequence;
	config_setting_lookup_string(hmackey, "key", &hmac_key_sequence);
	if(hmac_key_sequence == NULL)
		return PAM_SERVICE_ERR;

	uint8_t hmac_key_byte[8];
	to_byte_array(hmac_key_sequence,40,hmac_key_byte); /*14 = sizeof(key_[i].id)-1*/
	
	config_setting_t *db_ibutton;

	db_ibutton = config_lookup(&cfg, "ibutton_id_database");
	if(db_ibutton == NULL)
		return PAM_SERVICE_ERR;

	/* number of fields in config */
	unsigned int num_of_mem = config_setting_length(db_ibutton);
	/* array id */
	struct ibutton_keys_ key_[num_of_mem];
	/* hash result */
	struct ibutton_keys_ hash_key_[num_of_mem];
	uint8_t id_msg[7],hash[20];
	char interm_hash[65];

	/* move through the database fields */
	for(size_t i = 0;i < num_of_mem;++i){ 
		config_setting_t *nodes = config_setting_get_elem(db_ibutton, i);
		const char *section_name = config_setting_name(nodes); 

		/* scan filling of the fields */
		config_setting_lookup_string(db_ibutton, section_name, &(key_[i].id));

		/* convert hex string to byte in array for hmac */ 
		to_byte_array(key_[i].id,14,id_msg); /*14 = sizeof(key_[i].id)-1*/

		hmac_sha256(hash,hmac_key_byte,160,id_msg,56);

		for(int byte = 0;byte < 32; ++byte)
			sprintf(&interm_hash[byte*2], "%02x", hash[byte]);

		strcpy(hash_key_[i].hash_str,interm_hash);
		strcat(hash_key_[i].hash_str,"\n");
	}
	/*************************************************************************/

	enum serial_err error_out;

	static char *desired_port;

	if(NULL == list_ports())
		return PAM_SERVICE_ERR;

	desired_port = list_ports();

	char byte_buff[65];
	error_out = open_read_com(desired_port, attempts, _baudrate, byte_buff, hash_key_, num_of_mem, 65);

	if(E_UNPLUGGED == error_out)
		return PAM_USER_UNKNOWN;

	config_destroy(&cfg);

	return PAM_SUCCESS;
}

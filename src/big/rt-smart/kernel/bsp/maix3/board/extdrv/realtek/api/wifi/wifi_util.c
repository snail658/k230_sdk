#include <wireless.h>
#include <wlan_intf.h>
#include <basic_types.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_ind.h>
#include <osdep_service.h>

int iw_ioctl(const char * ifname, unsigned long request, struct iwreq *	pwrq)
{
	memcpy(pwrq->ifr_name, ifname, 5);
	return rltk_wlan_control(request, (void *) pwrq);
}

#ifdef CONFIG_CMW500_TEST
int wext_enable_testcmw500(const char *ifname, u8 fix_rate, u8 txpower, u8 pwrtrack_en)
{
	struct iwreq iwr;
	u8 para[3] = {0};
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	para[0] = fix_rate;
	para[1] = txpower;
	para[2] = pwrtrack_en;
	iwr.u.data.pointer = para;
	if (iw_ioctl(ifname, SOICSICMWTESTENABLE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}

int wext_disable_testcmw500(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	if (iw_ioctl(ifname, SOICSICMWTESTDISABLE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}
#endif
#ifdef CONFIG_RAWDATA
int wext_enable_rawdata_recv(const char *ifname, void *fun)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = fun;
	if (iw_ioctl(ifname, SIOCSIWRAWENABLE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}
int wext_disable_rawdata_recv(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	if (iw_ioctl(ifname, SOICSIWRAWDISABLE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}
int wext_send_rawdata(const char *ifname,const unsigned char* frame_buf, unsigned int frame_len)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = (void*)frame_buf;
	iwr.u.data.length = frame_len;
	if (iw_ioctl(ifname, SOICSIWDIRSEND, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}
#endif
int wext_get_ssid(const char *ifname, __u8 *ssid)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.essid.pointer = ssid;
	iwr.u.essid.length = 32;

	if (iw_ioctl(ifname, SIOCGIWESSID, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWESSID] ssid = NULL, not connected"); //do not use perror
		ret = -1;
	} else {
		ret = iwr.u.essid.length;
		if (ret > 32)
			ret = 32;
		/* Some drivers include nul termination in the SSID, so let's
		 * remove it here before further processing. WE-21 changes this
		 * to explicitly require the length _not_ to include nul
		 * termination. */
		if (ret > 0 && ssid[ret - 1] == '\0')
			ret--;
		ssid[ret] = '\0';
	}

	return ret;
}

int wext_set_ssid(const char *ifname, const __u8 *ssid, __u16 ssid_len)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.essid.pointer = (void *) ssid;
	iwr.u.essid.length = ssid_len;
	iwr.u.essid.flags = (ssid_len != 0);

	if (iw_ioctl(ifname, SIOCSIWESSID, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	
	return ret;
}

int wext_set_bssid(const char *ifname, const __u8 *bssid)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.ap_addr.sa_family = ARPHRD_ETHER;
	memcpy(iwr.u.ap_addr.sa_data, bssid, ETH_ALEN);

	if(bssid[ETH_ALEN]=='#' && bssid[ETH_ALEN + 1]=='@'){
		memcpy(iwr.u.ap_addr.sa_data + ETH_ALEN, bssid + ETH_ALEN, 6);
	}

	if (iw_ioctl(ifname, SIOCSIWAP, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWAP] error");
		ret = -1;
	}

	return ret;
}

int wext_get_bssid(const char*ifname, __u8 *bssid)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	if (iw_ioctl(ifname, SIOCGIWAP, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWAP] error");
		ret = -1;
	} else {
		memcpy(bssid, iwr.u.ap_addr.sa_data, ETH_ALEN);
    }

	return ret;
}

int is_broadcast_ether_addr(const unsigned char *addr)
{
	return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff;
}

int wext_set_auth_param(const char *ifname, __u16 idx, __u32 value)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.param.flags = idx & IW_AUTH_INDEX;
	iwr.u.param.value = value;

	if (iw_ioctl(ifname, SIOCSIWAUTH, &iwr) < 0) {
		DBG_INFO("WEXT: SIOCSIWAUTH(param %d value 0x%x) failed)", idx, value);
	}

	return ret;
}

int wext_set_mfp_support(const char *ifname, __u8 value)
{
	int ret = 0;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.param.value = value;

	if (iw_ioctl(ifname, SIOCSIWMFP, &iwr) < 0) {
		DBG_INFO("WEXT: SIOCSIWMFP(value 0x%x) failed)", value);
	}

	return ret;
}

#if CONFIG_SAE_SUPPORT
int wext_set_group_id(const char *ifname, __u8 value)
{
	int ret = 0;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.param.value = value;

	if (iw_ioctl(ifname, SIOCSIWGRPID, &iwr) < 0) {
		DBG_INFO("\n\rWEXT: SIOCSIWGRPID(value 0x%x) failed)", value);
	}

	return ret;
}
#endif

#if CONFIG_PMKSA_CACHING
int wext_set_pmk_cache_enable(const char *ifname, __u8 value)
{
	int ret = 0;
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.param.value = value;

	if (iw_ioctl(ifname, SIOCSIWPMKSA, &iwr) < 0) {
		DBG_INFO("\n\rWEXT: SIOCSIWPMKSA(value 0x%x) failed)", value);
	}

	return ret;
}
#endif

int wext_set_key_ext(const char *ifname, __u16 alg, const __u8 *addr, int key_idx, int set_tx, const __u8 *seq, __u16 seq_len, __u8 *key, __u16 key_len)
{
	struct iwreq iwr;
	int ret = 0;
	struct iw_encode_ext *ext;

	ext = (struct iw_encode_ext *) malloc(sizeof(struct iw_encode_ext) + key_len);
	if (ext == NULL)
		return -1;
	else
		memset(ext, 0, sizeof(struct iw_encode_ext) + key_len);

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.encoding.flags = key_idx + 1;
	iwr.u.encoding.flags |= IW_ENCODE_TEMP;
	iwr.u.encoding.pointer = ext;
	iwr.u.encoding.length = sizeof(struct iw_encode_ext) + key_len;

	if (alg == IW_ENCODE_DISABLED)
		iwr.u.encoding.flags |= IW_ENCODE_DISABLED;

	if (addr == NULL || is_broadcast_ether_addr(addr))
		ext->ext_flags |= IW_ENCODE_EXT_GROUP_KEY;

	if (set_tx)
		ext->ext_flags |= IW_ENCODE_EXT_SET_TX_KEY;

	ext->addr.sa_family = ARPHRD_ETHER;

	if (addr)
		memcpy(ext->addr.sa_data, addr, ETH_ALEN);
	else
		memset(ext->addr.sa_data, 0xff, ETH_ALEN);

	if (key && key_len) {
		memcpy(ext->key, key, key_len);
		ext->key_len = key_len;
	}

	ext->alg = alg;

	if (seq && seq_len) {
		ext->ext_flags |= IW_ENCODE_EXT_RX_SEQ_VALID;
		memcpy(ext->rx_seq, seq, seq_len);
	}

	if (iw_ioctl(ifname, SIOCSIWENCODEEXT, &iwr) < 0) {
		ret = -2;
		DBG_INFO("ioctl[SIOCSIWENCODEEXT] set key fail");
	}

	free(ext);

	return ret;
}

int wext_get_enc_ext(const char *ifname, __u16 *alg, __u8 *key_idx, __u8 *passphrase)
{
	struct iwreq iwr;
	int ret = 0;
	struct iw_encode_ext *ext;

	ext = (struct iw_encode_ext *) malloc(sizeof(struct iw_encode_ext) + 16);
	if (ext == NULL)
		return -1;
	else
		memset(ext, 0, sizeof(struct iw_encode_ext) + 16);

	iwr.u.encoding.pointer = ext;

	if (iw_ioctl(ifname, SIOCGIWENCODEEXT, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWENCODEEXT] error");
		ret = -1;
	}
	else
	{
		*alg = ext->alg;
		if(key_idx)
			*key_idx = (__u8)iwr.u.encoding.flags;
		if(passphrase)
			memcpy(passphrase, ext->key, ext->key_len);
	}

	if(ext != NULL)
		free(ext);
	
	return ret;
}

int wext_set_passphrase(const char *ifname, const __u8 *passphrase, __u16 passphrase_len)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.passphrase.pointer = (void *) passphrase;
	iwr.u.passphrase.length = passphrase_len;
	iwr.u.passphrase.flags = (passphrase_len != 0);

	if (iw_ioctl(ifname, SIOCSIWPRIVPASSPHRASE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID+0x1f] error");
		ret = -1;
	}
	
	return ret;
}

int wext_get_passphrase(const char *ifname, __u8 *passphrase)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.passphrase.pointer = (void *) passphrase;

	if (iw_ioctl(ifname, SIOCGIWPRIVPASSPHRASE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWPRIVPASSPHRASE] error");
		ret = -1;
	}
	else {
		ret = iwr.u.passphrase.length;
		passphrase[ret] = '\0';
	}
	
	return ret;
}

#if 0
int wext_set_mac_address(const char *ifname, char * mac)
{
	char buf[13+17+1];
	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 13+17, "write_mac %s", mac);
	return wext_private_command(ifname, buf, 0);
}

int wext_get_mac_address(const char *ifname, char * mac)
{
	int ret = 0;
	char buf[32];

	rtw_memset(buf, 0, sizeof(buf));
	rtw_memcpy(buf, "read_mac", 8);
	ret = wext_private_command_with_retval(ifname, buf, buf, 32);
	strcpy(mac, buf);
	return ret;
}
#endif

int wext_enable_powersave(const char *ifname, __u8 ips_mode, __u8 lps_mode)
{
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) + (1+1+1) );
	if(para == NULL) return -1;

	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 0; // type 0 for ips
	para[pindex++] = 1;
	para[pindex++] = ips_mode;

	para[pindex++] = 1; // type 1 for lps
	para[pindex++] = 1;
	para[pindex++] = lps_mode;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_disable_powersave(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) + (1+1+1) );
	if(para == NULL) return -1;

	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 0; // type 0 for ips
	para[pindex++] = 1;
	para[pindex++] = 0; // ips = 0

	para[pindex++] = 1; // type 1 for lps
	para[pindex++] = 1;
	para[pindex++] = 0; // lps = 0

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;

}

int wext_set_tdma_param(const char *ifname, __u8 slot_period, __u8 rfon_period_len_1, __u8 rfon_period_len_2, __u8 rfon_period_len_3)
{
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+4) );
	
	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 2; // type 2 tdma param
	para[pindex++] = 4;
	para[pindex++] = slot_period;
	para[pindex++] = rfon_period_len_1;
	para[pindex++] = rfon_period_len_2;
	para[pindex++] = rfon_period_len_3;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_set_lps_dtim(const char *ifname, __u8 lps_dtim)
{
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 3; // type 3 lps dtim
	para[pindex++] = 1;
	para[pindex++] = lps_dtim;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_get_lps_dtim(const char *ifname, __u8 *lps_dtim)
{

	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;
	
	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_get");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "pm_get");
	pindex = 7;

	para[pindex++] = 3; // type 3 for lps dtim
	para[pindex++] = 1;
	para[pindex++] = 0;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMGET] error");
		ret = -1;
		goto exit;
	}

	//get result at the beginning of iwr.u.data.pointer
	if((para[0]==3)&&(para[1]==1))
		*lps_dtim = para[2];
	else
		DBG_INFO("%s error", __func__);

exit:
	rtw_free(para);

	return ret;
}

int wext_set_lps_thresh(const char *ifname, u8 low_thresh) {
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 6; // type 6 lps threshold
	para[pindex++] = 1; // len
	para[pindex++] = low_thresh;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_set_beacon_mode(const char *ifname, __u8 mode) {
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 4; // type 4 beacon mode
	para[pindex++] = 1; // len
	para[pindex++] = mode;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_set_lps_level(const char *ifname, __u8 lps_level) {
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("pm_set");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( 7 + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "pm_set");
	pindex = 7;

	para[pindex++] = 5; // type 5 lps_level
	para[pindex++] = 1; // len
	para[pindex++] = lps_level;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVPMSET] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_set_tos_value(const char *ifname, __u8 *tos_value)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = sizeof("set_tos_value");

	memset(&iwr, 0, sizeof(iwr));

	para = rtw_malloc(cmd_len + 4);
	snprintf((char*)para, cmd_len, "set_tos_value");

	if((char)*tos_value >= 0 && *tos_value <=32){
		*(para + cmd_len)   = 0x4f;
		*(para + cmd_len+1) = 0xa4;
		*(para + cmd_len+2) = 0;
		*(para + cmd_len+3) = 0;
	}
	else if(*tos_value > 32 && *tos_value <=96){
		*(para + cmd_len)   = 0x2b;
		*(para + cmd_len+1) = 0xa4;
		*(para + cmd_len+2) = 0;
		*(para + cmd_len+3) = 0;
	}
	else if(*tos_value > 96 && *tos_value <= 160){
		*(para + cmd_len)   = 0x22;
		*(para + cmd_len+1) = 0x43;
		*(para + cmd_len+2) = 0x5e;
		*(para + cmd_len+3) = 0;
	}
	else if(*tos_value > 160){
		*(para + cmd_len)   = 0x22;
		*(para + cmd_len+1) = 0x32;
		*(para + cmd_len+2) = 0x2f;
		*(para + cmd_len+3) = 0;
	}

	iwr.u.data.pointer = para;
	iwr.u.data.length = cmd_len + 4;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_set_tos_value():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_get_tx_power(const char *ifname, __u8 *poweridx)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = sizeof("get_tx_power");

	memset(&iwr, 0, sizeof(iwr));
	//Tx power size : 20 Bytes
	//CCK 1M,2M,5.5M,11M : 4 Bytes
	//OFDM 6M, 9M, 12M, 18M, 24M, 36M 48M, 54M : 8 Bytes
	//MCS 0~7 : 8 Bytes
	para = rtw_malloc(cmd_len + 20);
	snprintf((char*)para, cmd_len, "get_tx_power");

	iwr.u.data.pointer = para;
	iwr.u.data.length = cmd_len + 20;
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_get_tx_power():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}

	memcpy(poweridx,(__u8 *)(iwr.u.data.pointer),20);
	rtw_free(para);
	return ret;
}

int wext_get_txpower(const char *ifname, u8 *poweridx)
{
	int ret = 0;

	ret = rltk_get_txpower(ifname,poweridx);

	return ret;
}

#if 0

int wext_set_txpower(const char *ifname, int poweridx)
{
	int ret = 0;
	char buf[24];
	
	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 24, "txpower patha=%d", poweridx);
	ret = wext_private_command(ifname, buf, 0);

	return ret;
}

int wext_get_associated_client_list(const char *ifname, void * client_list_buffer, uint16_t buffer_length)
{
	int ret = 0;
	char buf[25];

	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 25, "get_client_list %x", client_list_buffer);
	ret = wext_private_command(ifname, buf, 0);

	return ret;
}

int wext_get_ap_info(const char *ifname, rtw_bss_info_t * ap_info, rtw_security_t* security)
{
	int ret = 0;
	char buf[24];

	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 24, "get_ap_info %x", ap_info);
	ret = wext_private_command(ifname, buf, 0);

	snprintf(buf, 24, "get_security");
	ret = wext_private_command_with_retval(ifname, buf, buf, 24);
	sscanf(buf, "%d", security);

	return ret;
}
#endif

int wext_set_mode(const char *ifname, int mode)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.mode = mode;
	if (iw_ioctl(ifname, SIOCSIWMODE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWMODE] error");
		ret = -1;
	}

	return ret;
}

int wext_get_mode(const char *ifname, int *mode)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	if (iw_ioctl(ifname, SIOCGIWMODE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWMODE] error");
		ret = -1;
	}
	else
		*mode = iwr.u.mode;

	return ret;
}

int wext_set_ap_ssid(const char *ifname, const __u8 *ssid, __u16 ssid_len)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.essid.pointer = (void *) ssid;
	iwr.u.essid.length = ssid_len;
	iwr.u.essid.flags = (ssid_len != 0);

	if (iw_ioctl(ifname, SIOCSIWPRIVAPESSID, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVAPESSID] error");
		ret = -1;
	}
	
	return ret;
}

#if 0
int wext_set_country(const char *ifname, rtw_country_code_t country_code)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	iwr.u.param.value = country_code;
	
	if (iw_ioctl(ifname, SIOCSIWPRIVCOUNTRY, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVCOUNTRY] error");
		ret = -1;
	}
	return ret;
}
#endif
int wext_set_country(const char *ifname, u8 *country_code)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	iwr.u.data.pointer = country_code;
	
	if (iw_ioctl(ifname, SIOCSIWPRIVCOUNTRY, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVCOUNTRY] error");
		ret = -1;
	}
	return ret;
}

int wext_get_rssi(const char *ifname, int *rssi)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	if (iw_ioctl(ifname, SIOCGIWSENS, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWSENS] error");
		ret = -1;
	} else {
		*rssi = 0 - iwr.u.sens.value;
	}
	return ret;
}

int wext_set_pscan_channel(const char *ifname, __u8 *ch, __u8 *pscan_config, __u8 length)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int i =0;

	memset(&iwr, 0, sizeof(iwr));
	//Format of para:function_name num_channel chan1... pscan_config1 ...
	para = rtw_malloc((length + length + 1) + 12);//size:num_chan + num_time + length + function_name
	if(para == NULL) return -1;

	//Cmd
	snprintf((char*)para, 12, "PartialScan");
	//length
	*(para+12) = length;
	for(i = 0; i < length; i++){
		*(para + 13 + i)= *(ch + i);
		*((__u16*) (para + 13 + length + i))= *(pscan_config + i);
	}
	
	iwr.u.data.pointer = para;
	iwr.u.data.length = (length + length + 1) + 12;
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_set_pscan_channel():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	rtw_free(para);
	return ret;
}

int wext_set_scan_reorderchannel(const char *ifname, __u8 *ch, __u8 length)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int i =0;
	memset(&iwr, 0, sizeof(iwr));
	//Format of para:function_name num_channel chan1.....
	para = rtw_malloc((length + 1) + 12);//size:num_chan  + length + function_name
	if(para == NULL) 
		return -1;
	//Cmd
	snprintf((char*)para, 12, "ReorderScan");
	//length
	*(para+12) = length;
	for(i = 0; i < length; i++){
		*(para + 13 + i)= *(ch + i);
	}
	iwr.u.data.pointer = para;
	iwr.u.data.length = (length + 1) + 12;
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_set_pscan_channel():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	rtw_free(para);
	return ret;
}

int wext_set_channel(const char *ifname, __u8 ch)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.freq.m = 0;
	iwr.u.freq.e = 0;
	iwr.u.freq.i = ch;

	if (iw_ioctl(ifname, SIOCSIWFREQ, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWFREQ] error");
		ret = -1;
	}

	return ret;
}

int wext_get_channel(const char *ifname, __u8 *ch)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	if (iw_ioctl(ifname, SIOCGIWFREQ, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWFREQ] error");
		ret = -1;
	}
	else
		*ch = iwr.u.freq.i;

	return ret;
}
#if CONFIG_MULTICAST
int wext_register_multicast_address(const char *ifname, rtw_mac_t *mac)
{
	int ret = 0;
	char buf[32];

	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 32, "reg_multicast "MAC_FMT, MAC_ARG(mac->octet));
	ret = wext_private_command(ifname, buf, 0);

	return ret;
}

int wext_unregister_multicast_address(const char *ifname, rtw_mac_t *mac)
{
	int ret = 0;
	char buf[35];

	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 35, "reg_multicast -d "MAC_FMT, MAC_ARG(mac->octet));
	ret = wext_private_command(ifname, buf, 0);

	return ret;
}

int wext_enable_multicast_address_filter(const char *ifname)
{
	int ret = 0;
	char buf[32];

	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 32, "reg_multicast -f");
	buf[16] = 1;
	ret = wext_private_command(ifname, buf, 0);

	return ret;
}

int wext_disable_multicast_address_filter(const char *ifname)
{
	int ret = 0;
	char buf[35];

	rtw_memset(buf, 0, sizeof(buf));
	snprintf(buf, 35, "reg_multicast -f");
	buf[16] = 0;
	ret = wext_private_command(ifname, buf, 0);

	return ret;
}
#endif
int wext_set_scan(const char *ifname, char *buf, __u16 buf_len, __u16 flags)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
#if 0 //for scan_with_ssid	
	if(buf)
		memset(buf, 0, buf_len);
#endif
	iwr.u.data.pointer = buf;
	iwr.u.data.flags = flags;
	iwr.u.data.length = buf_len;
	if (iw_ioctl(ifname, SIOCSIWSCAN, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWSCAN] error");
		ret = -1;
	}
	return ret;
}

int wext_get_scan(const char *ifname, char *buf, __u16 buf_len)
{
	struct iwreq iwr;
	int ret = 0;

	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_len;
	if (iw_ioctl(ifname, SIOCGIWSCAN, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCGIWSCAN] error");
		ret = -1;
	}else
		ret = iwr.u.data.flags;
	return ret;
}
int wext_set_multiscan(const char *ifname, char *buf, __u16 buf_len, __u16 flags)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = buf;
	iwr.u.data.flags = flags;
	iwr.u.data.length = buf_len;
	if (iw_ioctl(ifname, SIOCSIWMULTISCAN, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWSCAN] error");
		ret = -1;
	}
	return ret;
}

int wext_private_command_with_retval(const char *ifname, char *cmd, char *ret_buf, int ret_len)
{
	struct iwreq iwr;
	int ret = 0, buf_size;
	char *buf;
	
	buf_size = 128;
	if(strlen(cmd) >= buf_size)
		buf_size = strlen(cmd) + 1;	// 1 : '\0'
	buf = (char*)rtw_malloc(buf_size);
	if(!buf){
		DBG_INFO("WEXT: Can't malloc memory");
		return -1;
	}
	memset(buf, 0, buf_size);
	strcpy(buf, cmd);
	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_size;
	iwr.u.data.flags = 0;

	if ((ret = iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr)) < 0) {
		DBG_INFO("ioctl[SIOCDEVPRIVATE] error. ret=%d\n", ret);
	}
	if(ret_buf){
		if(ret_len > iwr.u.data.length)
			ret_len =  iwr.u.data.length;
		rtw_memcpy(ret_buf, (char *) iwr.u.data.pointer, ret_len);
	}
	rtw_free(buf);
	return ret;
}

int wext_private_command(const char *ifname, char *cmd, int show_msg)
{
	struct iwreq iwr;
	int ret = 0, buf_size;
	char *buf;

	u8 cmdname[17] = {0}; // IFNAMSIZ+1

	sscanf(cmd, "%16s", cmdname);
	if((strcmp((const char *)cmdname, "config_get") == 0)
		|| (strcmp((const char *)cmdname, "config_set") == 0)
		|| (strcmp((const char *)cmdname, "efuse_get") == 0)
		|| (strcmp((const char *)cmdname, "efuse_set") == 0)
		|| (strcmp((const char *)cmdname, "mp_psd") == 0))
		buf_size = 2800;//2600 for config_get rmap,0,512 (or realmap)
	else
		buf_size = 512;
    


	if(strlen(cmd) >= buf_size)
		buf_size = strlen(cmd) + 1;	// 1 : '\0'
	buf = (char*)rtw_malloc(buf_size);
	if(!buf){
		DBG_INFO("WEXT: Can't malloc memory");
		return -1;
	}
	memset(buf, 0, buf_size);
	strcpy(buf, cmd);
	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_size;
	iwr.u.data.flags = 0;

	if ((ret = iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr)) < 0) {
		DBG_INFO("ioctl[SIOCDEVPRIVATE] error. ret=%d\n", ret);
	}
	if (show_msg && iwr.u.data.length) {
		if(iwr.u.data.length > buf_size)
			DBG_INFO("WEXT: Malloc memory is not enough");
		DBG_INFO("Private Message: %s", (char *) iwr.u.data.pointer);
	}
	rtw_free(buf);
	return ret;
}

void wext_wlan_indicate(unsigned int cmd, union iwreq_data *wrqu, char *extra)
{
	unsigned char null_mac[6] = {0};

	switch(cmd)
	{
		case SIOCGIWAP:
			if(wrqu->ap_addr.sa_family == ARPHRD_ETHER)
			{
				if(!memcmp(wrqu->ap_addr.sa_data, null_mac, sizeof(null_mac)))
					wifi_indication(WIFI_EVENT_DISCONNECT, wrqu->ap_addr.sa_data, sizeof(null_mac)+ 2, 0);
				else				
					wifi_indication(WIFI_EVENT_CONNECT, wrqu->ap_addr.sa_data, sizeof(null_mac), 0);
			}			
			break;

		case IWEVCUSTOM:
			if(extra)
			{
				if(!memcmp(IW_EXT_STR_FOURWAY_DONE, extra, strlen(IW_EXT_STR_FOURWAY_DONE)))
					wifi_indication(WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, extra, strlen(IW_EXT_STR_FOURWAY_DONE), 0);
				else if(!memcmp(IW_EXT_STR_RECONNECTION_FAIL, extra, strlen(IW_EXT_STR_RECONNECTION_FAIL)))
					wifi_indication(WIFI_EVENT_RECONNECTION_FAIL, extra, strlen(IW_EXT_STR_RECONNECTION_FAIL), 0);
				else if(!memcmp(IW_EVT_STR_NO_NETWORK, extra, strlen(IW_EVT_STR_NO_NETWORK)))
					wifi_indication(WIFI_EVENT_NO_NETWORK, extra, strlen(IW_EVT_STR_NO_NETWORK), 0);
				else if(!memcmp(IW_EVT_STR_ICV_ERROR, extra, strlen(IW_EVT_STR_ICV_ERROR)))
					wifi_indication(WIFI_EVENT_ICV_ERROR, extra, strlen(IW_EVT_STR_ICV_ERROR), 0);
				else if(!memcmp(IW_EVT_STR_CHALLENGE_FAIL, extra, strlen(IW_EVT_STR_CHALLENGE_FAIL)))
					wifi_indication(WIFI_EVENT_CHALLENGE_FAIL, extra, strlen(IW_EVT_STR_CHALLENGE_FAIL), 0);
				else if(!memcmp(IW_EVT_STR_SCAN_START, extra, strlen(IW_EVT_STR_SCAN_START)))
					wifi_indication(WIFI_EVENT_SCAN_START, extra, strlen(IW_EVT_STR_SCAN_START), 0);
				else if(!memcmp(IW_EVT_STR_SCAN_FAILED, extra, strlen(IW_EVT_STR_SCAN_FAILED)))
					wifi_indication(WIFI_EVENT_SCAN_FAILED, extra, strlen(IW_EVT_STR_SCAN_FAILED), 0);
				else if(!memcmp(IW_EVT_STR_AUTH, extra, strlen(IW_EVT_STR_AUTH)))
					wifi_indication(WIFI_EVENT_AUTHENTICATION, extra, strlen(IW_EVT_STR_AUTH), 0);
				else if(!memcmp(IW_EVT_STR_AUTH_REJECT, extra, strlen(IW_EVT_STR_AUTH_REJECT)))
					wifi_indication(WIFI_EVENT_AUTH_REJECT, extra, strlen(IW_EVT_STR_AUTH_REJECT), 0);
				else if(!memcmp(IW_EVT_STR_DEAUTH, extra, strlen(IW_EVT_STR_DEAUTH)))
					wifi_indication(WIFI_EVENT_DEAUTH, extra, strlen(IW_EVT_STR_DEAUTH), 0);
				else if(!memcmp(IW_EVT_STR_AUTH_TIMEOUT, extra, strlen(IW_EVT_STR_AUTH_TIMEOUT)))
					wifi_indication(WIFI_EVENT_AUTH_TIMEOUT, extra, strlen(IW_EVT_STR_AUTH_TIMEOUT), 0);
				else if(!memcmp(IW_EVT_STR_ASSOCIATING, extra, strlen(IW_EVT_STR_ASSOCIATING)))
					wifi_indication(WIFI_EVENT_ASSOCIATING, extra, strlen(IW_EVT_STR_ASSOCIATING), 0);
				else if(!memcmp(IW_EVT_STR_ASSOCIATED, extra, strlen(IW_EVT_STR_ASSOCIATED)))
					wifi_indication(WIFI_EVENT_ASSOCIATED, extra, strlen(IW_EVT_STR_ASSOCIATED), 0);
				else if(!memcmp(IW_EVT_STR_ASSOC_REJECT, extra, strlen(IW_EVT_STR_ASSOC_REJECT)))
					wifi_indication(WIFI_EVENT_ASSOC_REJECT, extra, strlen(IW_EVT_STR_ASSOC_REJECT), 0);
				else if(!memcmp(IW_EVT_STR_ASSOC_TIMEOUT, extra, strlen(IW_EVT_STR_ASSOC_TIMEOUT)))
					wifi_indication(WIFI_EVENT_ASSOC_TIMEOUT, extra, strlen(IW_EVT_STR_ASSOC_TIMEOUT), 0);
				else if(!memcmp(IW_EVT_STR_HANDSHAKE_FAILED, extra, strlen(IW_EVT_STR_HANDSHAKE_FAILED)))
					wifi_indication(WIFI_EVENT_HANDSHAKE_FAILED, extra, strlen(IW_EVT_STR_HANDSHAKE_FAILED), 0);
				else if(!memcmp(IW_EVT_STR_4Way_HANDSHAKE, extra, strlen(IW_EVT_STR_4Way_HANDSHAKE)))
					wifi_indication(WIFI_EVENT_4WAY_HANDSHAKE, extra, strlen(IW_EVT_STR_4Way_HANDSHAKE), 0);
				else if(!memcmp(IW_EVT_STR_GROUP_HANDSHAKE, extra, strlen(IW_EVT_STR_GROUP_HANDSHAKE)))
					wifi_indication(WIFI_EVENT_GROUP_HANDSHAKE, extra, strlen(IW_EVT_STR_GROUP_HANDSHAKE), 0);
				else if(!memcmp(IW_EVT_STR_GROUP_HANDSHAKE_DONE, extra, strlen(IW_EVT_STR_GROUP_HANDSHAKE_DONE)))
					wifi_indication(WIFI_EVENT_GROUP_HANDSHAKE_DONE, extra, strlen(IW_EVT_STR_GROUP_HANDSHAKE_DONE), 0);
				else if(!memcmp(IW_EVT_STR_CONN_TIMEOUT, extra, strlen(IW_EVT_STR_CONN_TIMEOUT)))
					wifi_indication(WIFI_EVENT_CONN_TIMEOUT, extra, strlen(IW_EVT_STR_CONN_TIMEOUT), 0);
#if CONFIG_ENABLE_P2P || CONFIG_AP_MODE
				else if(!memcmp(IW_EVT_STR_STA_ASSOC, extra, strlen(IW_EVT_STR_STA_ASSOC)))
					wifi_indication(WIFI_EVENT_STA_ASSOC, wrqu->data.pointer, wrqu->data.length, 0);
				else if(!memcmp(IW_EVT_STR_STA_DISASSOC, extra, strlen(IW_EVT_STR_STA_DISASSOC)))
					wifi_indication(WIFI_EVENT_STA_DISASSOC, wrqu->addr.sa_data, sizeof(null_mac), 0);
				else if(!memcmp(IW_EVT_STR_SEND_ACTION_DONE, extra, strlen(IW_EVT_STR_SEND_ACTION_DONE)))
					wifi_indication(WIFI_EVENT_SEND_ACTION_DONE, NULL, 0, wrqu->data.flags);
#endif			
#if CONFIG_BT_COEXIST
				else if(!memcmp(IW_EVT_STR_LEAVE_BUSY_TRAFFIC, extra, strlen(IW_EVT_STR_LEAVE_BUSY_TRAFFIC)))
					wifi_indication(WIFI_EVENT_LEAVE_BUSY_TRAFFIC, NULL, 0, 0);
#endif
			}
			break;
		case SIOCGIWSCAN:
			if(wrqu->data.pointer == NULL)
				wifi_indication(WIFI_EVENT_SCAN_DONE, NULL, 0, 0);
			else
				wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, wrqu->data.pointer, wrqu->data.length, 0);
			break;
		case IWEVMGNTRECV:
			wifi_indication(WIFI_EVENT_RX_MGNT, wrqu->data.pointer, wrqu->data.length, wrqu->data.flags);
			break;
#ifdef REPORT_STA_EVENT
		case IWEVREGISTERED:
			if(wrqu->addr.sa_family == ARPHRD_ETHER)
				wifi_indication(WIFI_EVENT_STA_ASSOC, wrqu->addr.sa_data, sizeof(null_mac), 0);
			break;
		case IWEVEXPIRED:
			if(wrqu->addr.sa_family == ARPHRD_ETHER)
				wifi_indication(WIFI_EVENT_STA_DISASSOC, wrqu->addr.sa_data, sizeof(null_mac), 0);
			break;
#endif
		default:
			break;

	}
	
}


int wext_send_eapol(const char *ifname, char *buf, __u16 buf_len, __u16 flags)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_len;
	iwr.u.data.flags = flags;	
	if (iw_ioctl(ifname, SIOCSIWEAPOLSEND, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWEAPOLSEND] error");
		ret = -1;
	}
	return ret;
}

int wext_send_mgnt(const char *ifname, char *buf, __u16 buf_len, __u16 flags)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_len;
	iwr.u.data.flags = flags;	
	if (iw_ioctl(ifname, SIOCSIWMGNTSEND, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWMGNTSEND] error");
		ret = -1;
	}
	return ret;
}

int wext_set_gen_ie(const char *ifname, char *buf, __u16 buf_len, __u16 flags)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));
	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_len;
	iwr.u.data.flags = flags;	
	if (iw_ioctl(ifname, SIOCSIWGENIE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWGENIE] error");
		ret = -1;
	}
	return ret;
}

int wext_set_autoreconnect(const char *ifname, __u8 mode, __u8 retry_times, __u16 timeout)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("SetAutoRecnt");
	para = rtw_malloc((4) + cmd_len);//size:para_len+cmd_len
	if(para == NULL) return -1;

	//Cmd
	snprintf((char*)para, cmd_len, "SetAutoRecnt");
	//length
	*(para+cmd_len) = mode;	//para1
	*(para+cmd_len+1) = retry_times; //para2
	*(para+cmd_len+2) = timeout; //para3
	
	iwr.u.data.pointer = para;
	iwr.u.data.length = (4) + cmd_len;
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_set_autoreconnect():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	rtw_free(para);
	return ret;
}

int wext_get_autoreconnect(const char *ifname, __u8 *mode)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("GetAutoRecnt");
	para = rtw_malloc(cmd_len);//size:para_len+cmd_len
	//Cmd
	snprintf((char*)para, cmd_len, "GetAutoRecnt");
	//length
	
	iwr.u.data.pointer = para;
	iwr.u.data.length = cmd_len;
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_get_autoreconnect():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	*mode = *(__u8 *)(iwr.u.data.pointer);
	rtw_free(para);
	return ret;
}

int wext_get_drv_ability(const char *ifname, __u32 *ability)
{
	int ret = 0;
	char * buf = (char *)rtw_zmalloc(33);
	if(buf == NULL) return -1;

	snprintf(buf, 33, "get_drv_ability %x", ability);
	ret = wext_private_command(ifname, buf, 0);

	rtw_free(buf);
	return ret;
}

#if CONFIG_CUSTOM_IE
int wext_add_custom_ie(const char *ifname, void *cus_ie, int ie_num)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;
	if(ie_num <= 0 || !cus_ie){
		DBG_INFO("wext_add_custom_ie():wrong parameter");
		ret = -1;
		return ret;
	}
	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("SetCusIE");
	para = rtw_malloc((4)* 2 + cmd_len);//size:addr len+cmd_len
	if(para == NULL) return -1;

	//Cmd
	snprintf((char*)para, cmd_len, "SetCusIE");
	//addr length
	*(__u32 *)(para + cmd_len) = (__u32)cus_ie; //ie addr
	//ie_num
	*(__u32 *)(para + cmd_len + 4) = ie_num; //num of ie

	iwr.u.data.pointer = para;
	iwr.u.data.length = (4)* 2 + cmd_len;// 2 input
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_add_custom_ie():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	rtw_free(para);

	return ret;
}

int wext_update_custom_ie(const char *ifname, void * cus_ie, int ie_index)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;
	if(ie_index <= 0 || !cus_ie){
		DBG_INFO("wext_update_custom_ie():wrong parameter");
		ret = -1;
		return ret;
	}
	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("UpdateIE");
	para = rtw_malloc((4)* 2 + cmd_len);//size:addr len+cmd_len
	if(para == NULL) return -1;

	//Cmd
	snprintf((char*)para, cmd_len, "UpdateIE");
	//addr length
	*(__u32 *)(para + cmd_len) = (__u32)cus_ie; //ie addr
	//ie_index
	*(__u32 *)(para + cmd_len + 4) = ie_index; //num of ie

	iwr.u.data.pointer = para;
	iwr.u.data.length = (4)* 2 + cmd_len;// 2 input
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_update_custom_ie():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	rtw_free(para);

	return ret;

}

int wext_del_custom_ie(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("DelIE");
	para = rtw_malloc(cmd_len);//size:addr len+cmd_len
	//Cmd
	snprintf((char*)para, cmd_len, "DelIE");
	
	iwr.u.data.pointer = para;
	iwr.u.data.length = cmd_len;
	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_del_custom_ie():ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}
	rtw_free(para);

	return ret;


}

#endif

extern void rltk_get_wlan_netifstat(const char *ifname, unsigned int* netinfo);
int wext_get_netinfo(const char *ifname, __u32* netinfo)
{
	rltk_get_wlan_netifstat(ifname,netinfo);
	return 0;
}

#if CONFIG_AP_MODE
int wext_enable_forwarding(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("forwarding_set");
	para = rtw_malloc(cmd_len + 1);
	if(para == NULL) return -1;

	// forwarding_set 1
	snprintf((char *) para, cmd_len, "forwarding_set");
	*(para + cmd_len) = '1';

	iwr.u.essid.pointer = para;
	iwr.u.essid.length = cmd_len + 1;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_enable_forwarding(): ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;
}

int wext_disable_forwarding(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("forwarding_set");
	para = rtw_malloc(cmd_len + 1);
	if(para == NULL) return -1;

	// forwarding_set 0
	snprintf((char *) para, cmd_len, "forwarding_set");
	*(para + cmd_len) = '0';

	iwr.u.essid.pointer = para;
	iwr.u.essid.length = cmd_len + 1;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("wext_disable_forwarding(): ioctl[SIOCDEVPRIVATE] error");
		ret = -1;
	}

	rtw_free(para);
	return ret;

}
#endif

#if CONFIG_CONCURRENT_MODE
int wext_set_ch_deauth(const char *ifname, __u8 enable)
{
	int ret = 0;
	char * buf = (char *)rtw_zmalloc(16);
	if(buf == NULL) return -1;

	snprintf(buf, 16, "SetChDeauth %d", enable);
	ret = wext_private_command(ifname, buf, 0);

	rtw_free(buf);
	return ret;
}
#endif

int wext_set_adaptivity(rtw_adaptivity_mode_t adaptivity_mode)
{
	extern u8 rtw_adaptivity_en;
	extern u8 rtw_adaptivity_mode;

	switch(adaptivity_mode){
		case RTW_ADAPTIVITY_NORMAL:
			rtw_adaptivity_en = 1; // enable adaptivity
			rtw_adaptivity_mode = RTW_ADAPTIVITY_MODE_NORMAL;
			break;
		case RTW_ADAPTIVITY_CARRIER_SENSE:
			rtw_adaptivity_en = 1; // enable adaptivity
			rtw_adaptivity_mode = RTW_ADAPTIVITY_MODE_CARRIER_SENSE;
			break;		
		case RTW_ADAPTIVITY_DISABLE:
		default:
			rtw_adaptivity_en = 0; //disable adaptivity
			break;
	}
	return 0;
}

int wext_set_adaptivity_th_l2h_ini(__u8 l2h_threshold)
{
	extern s8 rtw_adaptivity_th_l2h_ini;
	rtw_adaptivity_th_l2h_ini = (__s8)l2h_threshold;
	return 0;
}
#if CONFIG_ACS
extern int rltk_get_auto_chl(const char *ifname, unsigned char *channel_set, unsigned char channel_num);
int wext_get_auto_chl(const char *ifname, unsigned char *channel_set, unsigned char channel_num)
{
	int ret = -1;
	int channel = 0;
	//wext_disable_powersave(ifname);
	if((channel = rltk_get_auto_chl(ifname,channel_set,channel_num)) != 0 )
		ret = channel ;
	//wext_enable_powersave(ifname, 1, 1);
	return ret;
}
#endif
extern int rltk_set_sta_num(unsigned char ap_sta_num);
int wext_set_sta_num(unsigned char ap_sta_num)
{
	return rltk_set_sta_num(ap_sta_num);
}

extern int rltk_set_expire_time(unsigned int ap_expire_time);
int wext_set_expire_time(unsigned int ap_expire_time)
{
	return rltk_set_expire_time(ap_expire_time);
}

extern int rltk_del_station(const char *ifname, unsigned char* hwaddr);
int wext_del_station(const char *ifname, unsigned char* hwaddr)
{
	return rltk_del_station(ifname, hwaddr);
}

extern struct list_head *mf_list_head;
int wext_init_mac_filter(void)
{
	if(mf_list_head != NULL){
		return -1;
	}

	mf_list_head = (struct list_head *)malloc(sizeof(struct list_head));
	if(mf_list_head == NULL){
		DBG_INFO("[ERROR] %s : can't allocate mf_list_head",__func__);
		return -1;
	}

	INIT_LIST_HEAD(mf_list_head);

	return 0;
}

int wext_deinit_mac_filter(void)
{
	if(mf_list_head == NULL){
		return -1;
	}
	struct list_head *iterator;
	rtw_mac_filter_list_t *item;
	list_for_each(iterator, mf_list_head) {
		item = list_entry(iterator, rtw_mac_filter_list_t, node);
		list_del(iterator);
		free(item);
		item = NULL;
		iterator = mf_list_head;
	}

	free(mf_list_head);
	mf_list_head = NULL;
	return 0;
}

int wext_add_mac_filter(unsigned char* hwaddr)
{
	if(mf_list_head == NULL){
		return -1;
	}

	rtw_mac_filter_list_t *mf_list_new;
	mf_list_new =(rtw_mac_filter_list_t *) malloc(sizeof(rtw_mac_filter_list_t));
	if(mf_list_new == NULL){
		DBG_INFO("[ERROR] %s : can't allocate mf_list_new",__func__);
		return -1;
	}
	memcpy(mf_list_new->mac_addr,hwaddr,6);
	list_add(&(mf_list_new->node), mf_list_head);

	return 0;
}

int wext_del_mac_filter(unsigned char* hwaddr)
{
	if(mf_list_head == NULL){
		return -1;
	}

	struct list_head *iterator;
	rtw_mac_filter_list_t *item;
	list_for_each(iterator, mf_list_head) {
		item = list_entry(iterator, rtw_mac_filter_list_t, node);
		if(memcmp(item->mac_addr,hwaddr,6) == 0){
			list_del(iterator);
			free(item);
			item = NULL;
			return 0;
		}
	}
	return -1;
}

extern void rtw_set_indicate_mgnt(int enable);
void wext_set_indicate_mgnt(int enable)
{
	rtw_set_indicate_mgnt(enable);
	return;
}

#ifdef CONFIG_SW_MAILBOX_EN
int wext_mailbox_to_wifi(const char *ifname, char *buf, __u16 buf_len)
{
	struct iwreq iwr;
	int ret = 0;

	memset(&iwr, 0, sizeof(iwr));

	iwr.u.data.pointer = buf;
	iwr.u.data.length = buf_len;
	if (iw_ioctl(ifname, SIOCSIMAILBOX, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIMAILBOX] error");
		ret = -1;
	}
	return ret;
}
#endif

#if CONFIG_WOWLAN
int wext_wowlan_ctrl(const char *ifname, int enable){
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	DBG_INFO("wext_wowlan_ctrl: enable=%d\n\r", enable);
	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("wowlan_ctrl");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( cmd_len + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "wowlan_ctrl");
	pindex = cmd_len;

	para[pindex++] = 0; // type 0 wowlan enable disable
	para[pindex++] = 1;
	para[pindex++] = enable;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVWWCTL] error");
		ret = -1;
	}

	rtw_free(para);

	return 0;
}

int wext_wowlan_set_pattern(const char *ifname, wowlan_pattern_t pattern)
{
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("wowlan_ctrl");

	para = rtw_malloc( cmd_len + (1+1+1) + sizeof(pattern));
	snprintf((char*)para, cmd_len, "wowlan_ctrl");
	pindex = cmd_len;

	para[pindex++] = 1; // type 1 wowlan set pattern
	para[pindex++] = 2;
	para[pindex++] = sizeof(pattern);
	memcpy(&(para[pindex]), &pattern, sizeof(pattern));

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCDEVPRIVWWPTN] error");
		ret = -1;
	}

	rtw_free(para);
	
	return ret;
}

int wext_wlan_redl_fw(const char *ifname){
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	DBG_INFO("+ wext_wlan_redl_fw\n\r");
	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("wowlan_ctrl");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( cmd_len + (1+1) );
	
	snprintf((char*)para, cmd_len, "wowlan_ctrl");
	pindex = cmd_len;

	para[pindex++] = 2; // type 2 redownload fw
	para[pindex++] = 0;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVREDLFW] error");
		ret = -1;
	}

	rtw_free(para);

	return 0;
}

int wext_wowlan_unicast_wake_ctrl(const char *ifname, unsigned char enable){
	struct iwreq iwr;
	int ret = 0;
	__u16 pindex = 0;
	__u8 *para = NULL;
	int cmd_len = 0;

	DBG_INFO("wext_wowlan_unicast_wake_ctrl: enable=%d\n\r", enable);
	memset(&iwr, 0, sizeof(iwr));
	cmd_len = sizeof("wowlan_ctrl");

	// Encode parameters as TLV (type, length, value) format
	para = rtw_malloc( cmd_len + (1+1+1) );
	
	snprintf((char*)para, cmd_len, "wowlan_ctrl");
	pindex = cmd_len;

	para[pindex++] = 3; // type 3 wowlan unicast wake enable disable
	para[pindex++] = 1;
	para[pindex++] = enable;

	iwr.u.data.pointer = para;
	iwr.u.data.length = pindex;

	if (iw_ioctl(ifname, SIOCDEVPRIVATE, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWPRIVWWCTL] error");
		ret = -1;
	}

	rtw_free(para);

	return 0;
}

#endif

int wext_enable_adaptivity(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	iwr.u.param.value = 1;
	if (iw_ioctl(ifname, SIOCSIWPRIVADAPTIVITY, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}
int wext_disable_adaptivity(const char *ifname)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	if (iw_ioctl(ifname, SIOCSIWPRIVADAPTIVITY, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWESSID] error");
		ret = -1;
	}
	return ret;
}
#if CONFIG_PASSIVESCAN_HIDDENSSID_ENABLE
int wext_set_passivescan_hiddenssid(const char *ifname, unsigned char enable)
{
	struct iwreq iwr;
	int ret = 0;
	memset(&iwr, 0, sizeof(iwr));
	iwr.u.param.value = !!enable;
	if (iw_ioctl(ifname, SIOCSIWHIDDENSSIDSCAN, &iwr) < 0) {
		DBG_INFO("ioctl[SIOCSIWHIDDENSSIDSCAN] error");
		ret = -1;
	}
	return ret;
}
#endif
#if CONFIG_RW_PHYSICAL_EFUSE
extern unsigned char rtw_halmac_read_physical_efuse_byte(int idx_wlan, u16 addr, u16 cnts,u8 *data);
unsigned char wext_read_byte(int idx, unsigned short addr, unsigned short cnts, unsigned char *data)
{
	int err;
	err=rtw_halmac_read_physical_efuse_byte(idx, addr, cnts, data);
	if(err==_FAIL)
	{
			printk("rtw_halmac_read_physical_efuse_byte Fail!!\n");
			return _FAIL;
	}
	return  _SUCCESS;

}
#endif

int wext_get_max_support_data_rate(const char *ifname, unsigned char *prate,unsigned char *pfixed)
{
	return rltk_get_max_support_rate(ifname,prate,pfixed);
}
extern int rltk_get_rate(const char *ifname, unsigned char *prate, unsigned char *pshort_gi, unsigned char *pbandwidth);
int wext_get_real_time_data_rate_info(const char *ifname, unsigned char *prate, unsigned char *pshort_gi, unsigned char *pbandwidth)
{
	return rltk_get_rate(ifname, prate, pshort_gi, pbandwidth);
}
extern int rltk_get_snr(const char *ifname, unsigned char *ofdm_snr, unsigned char *ht_snr);
int wext_get_snr(const char *ifname, unsigned char *ofdm_snr, unsigned char *ht_snr)
{
	return rltk_get_snr(ifname, ofdm_snr, ht_snr);
}
extern int rltk_wlan_set_retry_limit(const char *ifname, unsigned char retry_limit);
int wext_set_retry_limit(const char *ifname, unsigned char retry_limit)
{
	return rltk_wlan_set_retry_limit(ifname, retry_limit);
}

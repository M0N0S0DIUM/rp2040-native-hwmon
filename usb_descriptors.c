#include "tusb.h"
#include <string.h>
#define VID 0x0c70
#define PID 0xf00a
#define EP_HID 0x81
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
static tusb_desc_device_t const dev={
 .bLength=sizeof(tusb_desc_device_t),.bDescriptorType=TUSB_DESC_DEVICE,.bcdUSB=0x0200,
 .bDeviceClass=0,.bDeviceSubClass=0,.bDeviceProtocol=0,.bMaxPacketSize0=64,
 .idVendor=VID,.idProduct=PID,.bcdDevice=0x0100,
 .iManufacturer=1,.iProduct=2,.iSerialNumber=3,.bNumConfigurations=1
};
static uint8_t const report_desc[]={TUD_HID_REPORT_DESC_GENERIC_INOUT(63,HID_REPORT_ID(1))};
static uint8_t const cfg[]={
 TUD_CONFIG_DESCRIPTOR(1,1,0,CONFIG_TOTAL_LEN,0,100),
 TUD_HID_DESCRIPTOR(0,0,HID_ITF_PROTOCOL_NONE,sizeof(report_desc),EP_HID,64,10)
};
uint8_t const*tud_descriptor_device_cb(void){return(uint8_t const*)&dev;}
uint8_t const*tud_descriptor_configuration_cb(uint8_t index){(void)index;return cfg;}
uint8_t const*tud_hid_descriptor_report_cb(uint8_t instance){(void)instance;return report_desc;}
static const char*strs[]={"","RP2040 experimental","Farbwerk temperature prototype","RP2040-NTC-01"};
static uint16_t desc_str[64];
uint16_t const*tud_descriptor_string_cb(uint8_t index,uint16_t langid){
 (void)langid;
 if(index==0){desc_str[0]=(TUSB_DESC_STRING<<8)|4;desc_str[1]=0x0409;return desc_str;}
 if(index>=4)return NULL;
 size_t len=strlen(strs[index]);if(len>63)len=63;
 desc_str[0]=(TUSB_DESC_STRING<<8)|(2*len+2);
 for(size_t i=0;i<len;i++)desc_str[1+i]=strs[index][i];
 return desc_str;
}
uint16_t tud_hid_get_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t type,uint8_t*buffer,uint16_t reqlen){
 (void)instance;(void)report_id;(void)type;(void)buffer;(void)reqlen;return 0;
}
void tud_hid_set_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t type,uint8_t const*buffer,uint16_t len){
 (void)instance;(void)report_id;(void)type;(void)buffer;(void)len;
}

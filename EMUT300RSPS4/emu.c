/*
 Copyright (c) 2019 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "../adapter_common.c"

const uint8_t PROGMEM buff3[] = {
        0xF3, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00
};

const uint8_t PROGMEM buf03[] = {
        0x03, 0x21, 0x27, 0x03, 0x11, 0x06, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x0D, 0x0D, 0x00, 0x00, 0x00, 0x00,
        0x0D, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t PROGMEM buf4f[] = {
        0x4f, 0xc9, 0xa8, 0xb6, 0x15
};

const uint8_t PROGMEM buf4e[] = {
        0x4e, 0x14
};

const uint8_t PROGMEM buf4d[] = {
        0x4d, 0xe8, 0x03
};

const uint8_t PROGMEM buf4c[] = {
        0x4c, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00
};

static uint8_t buf4b[] = {
        0x4b, 0x54, 0xd5, 0x80, 0x00, 0x00, 0x00, 0x00
};

void EVENT_USB_Device_ControlRequest(void) {

    static uint8_t buffer[MAX_CONTROL_TRANSFER_SIZE];

    switch (USB_ControlRequest.bRequest) {
    case REQ_GetReport:
        if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
            uint8_t reportType = USB_ControlRequest.wValue >> 8;
            uint8_t reportId = USB_ControlRequest.wValue & 0xff;

            if (reportType == REPORT_TYPE_FEATURE) {
                const void *feature = NULL;
                uint8_t len = 0;

                switch (reportId) {
                case 0xf1:
                case 0xf2:
                    spoofReply = 0;
                    send_spoof_header();
                    while (!spoofReply)
                        ;
                    Endpoint_ClearSETUP();
                    Endpoint_Write_Control_Stream_LE(buf, spoofReplyLen);
                    Endpoint_ClearOUT();
                    break;
                case 0x03:
                    feature = buf03;
                    len = sizeof(buf03);
                    break;
                case 0xf3:
                    feature = buff3;
                    len = sizeof(buff3);
                    break;
                case 0x4b:
                    Endpoint_ClearSETUP();
                    Endpoint_Write_Control_Stream_LE(buf4b, sizeof(buf4b));
                    Endpoint_ClearOUT();
                    Serial_SendByte(BYTE_DEBUG);
                    Serial_SendByte(2);
                    Serial_SendByte(reportId);
                    Serial_SendByte(sizeof(buf4b));
                    break;
                case 0x4c:
                    feature = buf4c;
                    len = sizeof(buf4c);
                    break;
                case 0x4d:
                    feature = buf4d;
                    len = sizeof(buf4d);
                    break;
                case 0x4e:
                    feature = buf4e;
                    len = sizeof(buf4e);
                    break;
                case 0x4f:
                    feature = buf4f;
                    len = sizeof(buf4f);
                    break;
                default:
                    Serial_SendByte(BYTE_DEBUG);
                    Serial_SendByte(BYTE_LEN_1_BYTE);
                    Serial_SendByte(reportId);
                    break;
                }

                if (feature) {
                    Endpoint_ClearSETUP();
                    Endpoint_Write_Control_PStream_LE(feature, len);
                    Endpoint_ClearOUT();

                    Serial_SendByte(BYTE_DEBUG);
                    Serial_SendByte(2);
                    Serial_SendByte(reportId);
                    Serial_SendByte(len);
                }
            }
        }
        break;
    case REQ_SetReport:
        if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
            Endpoint_ClearSETUP();
            Endpoint_Read_Control_Stream_LE(buffer, USB_ControlRequest.wLength);
            Endpoint_ClearIN();

            uint8_t reportType = USB_ControlRequest.wValue >> 8;
            uint8_t reportId = USB_ControlRequest.wValue & 0xff;

            if (reportType == REPORT_TYPE_FEATURE) {
                switch (reportId) {
                case 0xf0:
                    send_spoof_header();
                    Serial_SendData(buffer, USB_ControlRequest.wLength);
                    break;
                default:
                    Serial_SendByte(BYTE_DEBUG);
                    Serial_SendByte(sizeof(USB_ControlRequest) + (USB_ControlRequest.wLength & 0xFF));
                    Serial_SendData(&USB_ControlRequest, sizeof(USB_ControlRequest));
                    Serial_SendData(buffer, USB_ControlRequest.wLength);
                    break;
                }
            }
        }
        break;
    }
}

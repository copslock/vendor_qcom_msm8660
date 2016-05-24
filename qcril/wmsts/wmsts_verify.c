
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*


                    W I R E L E S S   M E S S A G I N G   S E R V I C E S
                   -- Translation Services for CDMA SMS

GENERAL DESCRIPTION
  Verifies that RIL CDMA interface is compatible with WMS.

Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 by Qualcomm Technologies, Inc.
  All Rights Reserved.

Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.


/* ^L<EJECT> */
/*===========================================================================

$Header:

===========================================================================*/


/*===========================================================================
========================  INCLUDE FILES =====================================
===========================================================================*/



#include "comdef.h"
#include "wms.h"
#include "wmsts.h"
#include "ril.h"
#include "ril_cdma_sms.h"

#define WMSTS_VERIFY( xx_exp ) if (!(xx_exp)) {return 1;}

/*=========================================================================
FUNCTION
  wmsts_verify

DESCRIPTION
  Verifies RIL_CDMA_SMS_ClientBd does not differ from wms_client_bd_s_type.
  This is important since QCRIL takes a RIL_CDMA_SMS_ClientBd type and
  casts it to wms_client_bd_s_type to pass it into the wmsts library.
  This check is currently limited to detecting changes in ENUM values or
  sizes of any underlying data structures.  Any changes to the RIL_CDMA_SMS
  types in ril.h should be investigated specifically to make sure they 
  won't break the wmsts library.

DEPENDENCIES
  None

RETURN VALUE
  0 == SUCCESS, 1 == FAILURE

SIDE EFFECTS
  None
=========================================================================*/
int wmsts_verify( void )
{
  WMSTS_VERIFY (WMS_UDH_MAX_SND_SIZE == RIL_CDMA_SMS_UDH_MAX_SND_SIZE);
  WMSTS_VERIFY (WMS_UDH_EO_DATA_SEGMENT_MAX == RIL_CDMA_SMS_UDH_EO_DATA_SEGMENT_MAX);
  WMSTS_VERIFY (WMS_MAX_UD_HEADERS == RIL_CDMA_SMS_MAX_UD_HEADERS);
  WMSTS_VERIFY (WMS_CDMA_USER_DATA_MAX == RIL_CDMA_SMS_USER_DATA_MAX);
  WMSTS_VERIFY (WMS_ADDRESS_MAX == RIL_CDMA_SMS_ADDRESS_MAX);
  WMSTS_VERIFY (WMS_UDH_LARGE_PIC_SIZE == RIL_CDMA_SMS_UDH_LARGE_PIC_SIZE);
  WMSTS_VERIFY (WMS_UDH_SMALL_PIC_SIZE == RIL_CDMA_SMS_UDH_SMALL_PIC_SIZE);
  WMSTS_VERIFY (WMS_UDH_VAR_PIC_SIZE == RIL_CDMA_SMS_UDH_VAR_PIC_SIZE);
  WMSTS_VERIFY (WMS_UDH_ANIM_NUM_BITMAPS == RIL_CDMA_SMS_UDH_ANIM_NUM_BITMAPS);
  WMSTS_VERIFY (WMS_UDH_LARGE_BITMAP_SIZE == RIL_CDMA_SMS_UDH_LARGE_BITMAP_SIZE);
  WMSTS_VERIFY (WMS_UDH_SMALL_BITMAP_SIZE == RIL_CDMA_SMS_UDH_SMALL_BITMAP_SIZE);
  WMSTS_VERIFY (WMS_UDH_OTHER_SIZE == RIL_CDMA_SMS_UDH_OTHER_SIZE);
  WMSTS_VERIFY (WMS_UDH_CONCAT_8 == RIL_CDMA_SMS_UDH_CONCAT_8);
  WMSTS_VERIFY (WMS_UDH_SPECIAL_SM == RIL_CDMA_SMS_UDH_SPECIAL_SM);
  WMSTS_VERIFY (WMS_UDH_PORT_8 == RIL_CDMA_SMS_UDH_PORT_8);
  WMSTS_VERIFY (WMS_UDH_PORT_16 == RIL_CDMA_SMS_UDH_PORT_16);
  WMSTS_VERIFY (WMS_UDH_SMSC_CONTROL == RIL_CDMA_SMS_UDH_SMSC_CONTROL);
  WMSTS_VERIFY (WMS_UDH_SOURCE == RIL_CDMA_SMS_UDH_SOURCE);
  WMSTS_VERIFY (WMS_UDH_CONCAT_16 == RIL_CDMA_SMS_UDH_CONCAT_16);
  WMSTS_VERIFY (WMS_UDH_WCMP == RIL_CDMA_SMS_UDH_WCMP);
  WMSTS_VERIFY (WMS_UDH_TEXT_FORMATING == RIL_CDMA_SMS_UDH_TEXT_FORMATING);
  WMSTS_VERIFY (WMS_UDH_PRE_DEF_SOUND == RIL_CDMA_SMS_UDH_PRE_DEF_SOUND);
  WMSTS_VERIFY (WMS_UDH_USER_DEF_SOUND == RIL_CDMA_SMS_UDH_USER_DEF_SOUND);
  WMSTS_VERIFY (WMS_UDH_PRE_DEF_ANIM == RIL_CDMA_SMS_UDH_PRE_DEF_ANIM);
  WMSTS_VERIFY (WMS_UDH_LARGE_ANIM == RIL_CDMA_SMS_UDH_LARGE_ANIM);
  WMSTS_VERIFY (WMS_UDH_SMALL_ANIM == RIL_CDMA_SMS_UDH_SMALL_ANIM);
  WMSTS_VERIFY (WMS_UDH_LARGE_PICTURE == RIL_CDMA_SMS_UDH_LARGE_PICTURE);
  WMSTS_VERIFY (WMS_UDH_SMALL_PICTURE == RIL_CDMA_SMS_UDH_SMALL_PICTURE);
  WMSTS_VERIFY (WMS_UDH_VAR_PICTURE == RIL_CDMA_SMS_UDH_VAR_PICTURE);
  WMSTS_VERIFY (WMS_UDH_USER_PROMPT == RIL_CDMA_SMS_UDH_USER_PROMPT);
  WMSTS_VERIFY (WMS_UDH_EXTENDED_OBJECT == RIL_CDMA_SMS_UDH_EXTENDED_OBJECT);
  WMSTS_VERIFY (WMS_UDH_RFC822 == RIL_CDMA_SMS_UDH_RFC822);
  WMSTS_VERIFY (WMS_UDH_OTHER == RIL_CDMA_SMS_UDH_OTHER);
  WMSTS_VERIFY (WMS_UDH_ID_MAX32 == RIL_CDMA_SMS_UDH_ID_MAX32);
  WMSTS_VERIFY( sizeof (wms_udh_id_e_type) == sizeof (RIL_CDMA_SMS_UdhId) );
  WMSTS_VERIFY( sizeof (wms_udh_concat_8_s_type) == sizeof (RIL_CDMA_SMS_UdhConcat8) );
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_NONE == RIL_CDMA_SMS_GW_MSG_WAITING_NONE);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_DISCARD == RIL_CDMA_SMS_GW_MSG_WAITING_DISCARD);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_STORE == RIL_CDMA_SMS_GW_MSG_WAITING_STORE);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_NONE_1111 == RIL_CDMA_SMS_GW_MSG_WAITING_NONE_1111);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_MAX32 == RIL_CDMA_SMS_GW_MSG_WAITING_MAX32);
  WMSTS_VERIFY( sizeof (wms_gw_msg_waiting_e_type) == sizeof (RIL_CDMA_SMS_GWMsgWaiting) );
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_VOICEMAIL == RIL_CDMA_SMS_GW_MSG_WAITING_VOICEMAIL);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_FAX == RIL_CDMA_SMS_GW_MSG_WAITING_FAX);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_EMAIL == RIL_CDMA_SMS_GW_MSG_WAITING_EMAIL);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_OTHER == RIL_CDMA_SMS_GW_MSG_WAITING_OTHER);
  WMSTS_VERIFY (WMS_GW_MSG_WAITING_KIND_MAX32 == RIL_CDMA_SMS_GW_MSG_WAITING_KIND_MAX32);
  WMSTS_VERIFY( sizeof (wms_gw_msg_waiting_kind_e_type) == sizeof (RIL_CDMA_SMS_GWMsgWaitingKind) );
  WMSTS_VERIFY( sizeof (wms_udh_special_sm_s_type) == sizeof (RIL_CDMA_SMS_UdhSpecialSM) );
  WMSTS_VERIFY( sizeof (wms_udh_wap_8_s_type) == sizeof (RIL_CDMA_SMS_UdhWap8) );
  WMSTS_VERIFY( sizeof (wms_udh_wap_16_s_type) == sizeof (RIL_CDMA_SMS_UdhWap16) );
  WMSTS_VERIFY( sizeof (wms_udh_concat_16_s_type) == sizeof (RIL_CDMA_SMS_UdhConcat16) );
  WMSTS_VERIFY (WMS_UDH_LEFT_ALIGNMENT == RIL_CDMA_SMS_UDH_LEFT_ALIGNMENT);
  WMSTS_VERIFY (WMS_UDH_CENTER_ALIGNMENT == RIL_CDMA_SMS_UDH_CENTER_ALIGNMENT);
  WMSTS_VERIFY (WMS_UDH_RIGHT_ALIGNMENT == RIL_CDMA_SMS_UDH_RIGHT_ALIGNMENT);
  WMSTS_VERIFY (WMS_UDH_DEFAULT_ALIGNMENT == RIL_CDMA_SMS_UDH_DEFAULT_ALIGNMENT);
  WMSTS_VERIFY (WMS_UDH_MAX_ALIGNMENT == RIL_CDMA_SMS_UDH_MAX_ALIGNMENT);
  WMSTS_VERIFY (WMS_UDH_ALIGNMENT_MAX32 == RIL_CDMA_SMS_UDH_ALIGNMENT_MAX32);
  WMSTS_VERIFY( sizeof (wms_udh_alignment_e_type) == sizeof (RIL_CDMA_SMS_UdhAlignment) );
  WMSTS_VERIFY (WMS_UDH_FONT_NORMAL == RIL_CDMA_SMS_UDH_FONT_NORMAL);
  WMSTS_VERIFY (WMS_UDH_FONT_LARGE == RIL_CDMA_SMS_UDH_FONT_LARGE);
  WMSTS_VERIFY (WMS_UDH_FONT_SMALL == RIL_CDMA_SMS_UDH_FONT_SMALL);
  WMSTS_VERIFY (WMS_UDH_FONT_RESERVED == RIL_CDMA_SMS_UDH_FONT_RESERVED);
  WMSTS_VERIFY (WMS_UDH_FONT_MAX == RIL_CDMA_SMS_UDH_FONT_MAX);
  WMSTS_VERIFY (WMS_UDH_FONT_MAX32 == RIL_CDMA_SMS_UDH_FONT_MAX32);
  WMSTS_VERIFY( sizeof (wms_udh_font_size_e_type) == sizeof (RIL_CDMA_SMS_UdhFontSize) );
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BLACK == RIL_CDMA_SMS_UDH_TEXT_COLOR_BLACK);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_GREY == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_GREY);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_RED == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_RED);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_YELLOW == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_YELLOW);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_GREEN == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_GREEN);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_CYAN == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_CYAN);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_BLUE == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_BLUE);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_DARK_MAGENTA == RIL_CDMA_SMS_UDH_TEXT_COLOR_DARK_MAGENTA);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_GREY == RIL_CDMA_SMS_UDH_TEXT_COLOR_GREY);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_WHITE == RIL_CDMA_SMS_UDH_TEXT_COLOR_WHITE);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BRIGHT_RED == RIL_CDMA_SMS_UDH_TEXT_COLOR_BRIGHT_RED);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BRIGHT_YELLOW == RIL_CDMA_SMS_UDH_TEXT_COLOR_BRIGHT_YELLOW);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BRIGHT_GREEN == RIL_CDMA_SMS_UDH_TEXT_COLOR_BRIGHT_GREEN);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BRIGHT_CYAN == RIL_CDMA_SMS_UDH_TEXT_COLOR_BRIGHT_CYAN);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BRIGHT_BLUE == RIL_CDMA_SMS_UDH_TEXT_COLOR_BRIGHT_BLUE);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_BRIGHT_MAGENTA == RIL_CDMA_SMS_UDH_TEXT_COLOR_BRIGHT_MAGENTA);
  WMSTS_VERIFY (WMS_UDH_TEXT_COLOR_MAX32 == RIL_CDMA_SMS_UDH_TEXT_COLOR_MAX32);
  WMSTS_VERIFY( sizeof (wms_udh_text_color_e_type) == sizeof (RIL_CDMA_SMS_UdhTextColor) );
  WMSTS_VERIFY( sizeof (wms_udh_text_formating_s_type) == sizeof (RIL_CDMA_SMS_UdhTextFormating) );
  WMSTS_VERIFY( sizeof (wms_udh_pre_def_sound_s_type) == sizeof (RIL_CDMA_SMS_UdhPreDefSound) );
  WMSTS_VERIFY( sizeof (wms_udh_user_def_sound_s_type) == sizeof (RIL_CDMA_SMS_UdhUserDefSound) );
  WMSTS_VERIFY( sizeof (wms_udh_large_picture_data_s_type) == sizeof (RIL_CDMA_SMS_UdhLargePictureData) );
  WMSTS_VERIFY( sizeof (wms_udh_small_picture_data_s_type) == sizeof (RIL_CDMA_SMS_UdhSmallPictureData) );
  WMSTS_VERIFY( sizeof (wms_udh_var_picture_s_type) == sizeof (RIL_CDMA_SMS_UdhVarPicture) );
  WMSTS_VERIFY( sizeof (wms_udh_pre_def_anim_s_type) == sizeof (RIL_CDMA_SMS_UdhPreDefAnim) );
  WMSTS_VERIFY( sizeof (wms_udh_large_anim_s_type) == sizeof (RIL_CDMA_SMS_UdhLargeAnim) );
  WMSTS_VERIFY( sizeof (wms_udh_small_anim_s_type) == sizeof (RIL_CDMA_SMS_UdhSmallAnim) );
  WMSTS_VERIFY( sizeof (wms_udh_user_prompt_s_type) == sizeof (RIL_CDMA_SMS_UdhUserPrompt) );
  WMSTS_VERIFY( sizeof (wms_udh_eo_content_s_type) == sizeof (RIL_CDMA_SMS_UdhEoContent) );
  WMSTS_VERIFY (WMS_UDH_EO_VCARD == RIL_CDMA_SMS_UDH_EO_VCARD);
  WMSTS_VERIFY (WMS_UDH_EO_VCALENDAR == RIL_CDMA_SMS_UDH_EO_VCALENDAR);
  WMSTS_VERIFY (WMS_UDH_EO_MAX32 == RIL_CDMA_SMS_UDH_EO_MAX32);
  WMSTS_VERIFY( sizeof (wms_udh_eo_id_e_type) == sizeof (RIL_CDMA_SMS_UdhEoId) );
  WMSTS_VERIFY( sizeof (wms_udh_eo_s_type) == sizeof (RIL_CDMA_SMS_UdhEo) );
  WMSTS_VERIFY( sizeof (wms_udh_id_e_type) == sizeof (RIL_CDMA_SMS_UdhId) );
  WMSTS_VERIFY( sizeof (wms_udh_other_s_type) == sizeof (RIL_CDMA_SMS_UdhOther) );
  WMSTS_VERIFY( sizeof (wms_udh_rfc822_s_type) == sizeof (RIL_CDMA_SMS_UdhRfc822) );
  WMSTS_VERIFY( sizeof (wms_udh_s_type) == sizeof (RIL_CDMA_SMS_Udh) );
  WMSTS_VERIFY (WMS_ENCODING_OCTET == RIL_CDMA_SMS_ENCODING_OCTET);
  WMSTS_VERIFY (WMS_ENCODING_IS91EP == RIL_CDMA_SMS_ENCODING_IS91EP);
  WMSTS_VERIFY (WMS_ENCODING_ASCII == RIL_CDMA_SMS_ENCODING_ASCII);
  WMSTS_VERIFY (WMS_ENCODING_IA5 == RIL_CDMA_SMS_ENCODING_IA5);
  WMSTS_VERIFY (WMS_ENCODING_UNICODE == RIL_CDMA_SMS_ENCODING_UNICODE);
  WMSTS_VERIFY (WMS_ENCODING_SHIFT_JIS == RIL_CDMA_SMS_ENCODING_SHIFT_JIS);
  WMSTS_VERIFY (WMS_ENCODING_KOREAN == RIL_CDMA_SMS_ENCODING_KOREAN);
  WMSTS_VERIFY (WMS_ENCODING_LATIN_HEBREW == RIL_CDMA_SMS_ENCODING_LATIN_HEBREW);
  WMSTS_VERIFY (WMS_ENCODING_LATIN == RIL_CDMA_SMS_ENCODING_LATIN);
  WMSTS_VERIFY (WMS_ENCODING_GSM_7_BIT_DEFAULT == RIL_CDMA_SMS_ENCODING_GSM_7_BIT_DEFAULT);
  WMSTS_VERIFY (WMS_ENCODING_MAX32 == RIL_CDMA_SMS_ENCODING_MAX32);
  WMSTS_VERIFY( sizeof (wms_user_data_encoding_e_type) == sizeof (RIL_CDMA_SMS_UserDataEncoding) );
  WMSTS_VERIFY (WMS_IS91EP_VOICE_MAIL == RIL_CDMA_SMS_IS91EP_VOICE_MAIL);
  WMSTS_VERIFY (WMS_IS91EP_SHORT_MESSAGE_FULL == RIL_CDMA_SMS_IS91EP_SHORT_MESSAGE_FULL);
  WMSTS_VERIFY (WMS_IS91EP_CLI_ORDER == RIL_CDMA_SMS_IS91EP_CLI_ORDER);
  WMSTS_VERIFY (WMS_IS91EP_SHORT_MESSAGE == RIL_CDMA_SMS_IS91EP_SHORT_MESSAGE);
  WMSTS_VERIFY (WMS_IS91EP_MAX32 == RIL_CDMA_SMS_IS91EP_MAX32);
  WMSTS_VERIFY( sizeof (wms_IS91EP_type_e_type) == sizeof (RIL_CDMA_SMS_IS91EPType) );
  WMSTS_VERIFY( sizeof (wms_cdma_user_data_s_type) == sizeof (RIL_CDMA_SMS_CdmaUserData) );
  WMSTS_VERIFY (WMS_BD_TYPE_RESERVED_0 == RIL_CDMA_SMS_BD_TYPE_RESERVED_0);
  WMSTS_VERIFY (WMS_BD_TYPE_DELIVER == RIL_CDMA_SMS_BD_TYPE_DELIVER);
  WMSTS_VERIFY (WMS_BD_TYPE_SUBMIT == RIL_CDMA_SMS_BD_TYPE_SUBMIT);
  WMSTS_VERIFY (WMS_BD_TYPE_CANCELLATION == RIL_CDMA_SMS_BD_TYPE_CANCELLATION);
  WMSTS_VERIFY (WMS_BD_TYPE_DELIVERY_ACK == RIL_CDMA_SMS_BD_TYPE_DELIVERY_ACK);
  WMSTS_VERIFY (WMS_BD_TYPE_USER_ACK == RIL_CDMA_SMS_BD_TYPE_USER_ACK);
  WMSTS_VERIFY (WMS_BD_TYPE_READ_ACK == RIL_CDMA_SMS_BD_TYPE_READ_ACK);
  WMSTS_VERIFY (WMS_BD_TYPE_MAX32 == RIL_CDMA_SMS_BD_TYPE_MAX32);
  WMSTS_VERIFY( sizeof (wms_bd_message_type_e_type) == sizeof (RIL_CDMA_SMS_BdMessageType) );
  WMSTS_VERIFY( sizeof (wms_message_number_type) == sizeof (RIL_CDMA_SMS_MessageNumber) );
  WMSTS_VERIFY( sizeof (wms_message_id_s_type) == sizeof (RIL_CDMA_SMS_MessageId) );
  WMSTS_VERIFY( sizeof (wms_user_response_type) == sizeof (RIL_CDMA_SMS_UserResponse) );
  WMSTS_VERIFY( sizeof (wms_timestamp_s_type) == sizeof (RIL_CDMA_SMS_Timestamp) );
  WMSTS_VERIFY (WMS_PRIORITY_NORMAL == RIL_CDMA_SMS_PRIORITY_NORMAL);
  WMSTS_VERIFY (WMS_PRIORITY_INTERACTIVE == RIL_CDMA_SMS_PRIORITY_INTERACTIVE);
  WMSTS_VERIFY (WMS_PRIORITY_URGENT == RIL_CDMA_SMS_PRIORITY_URGENT);
  WMSTS_VERIFY (WMS_PRIORITY_EMERGENCY == RIL_CDMA_SMS_PRIORITY_EMERGENCY);
  WMSTS_VERIFY (WMS_PRIORITY_MAX32 == RIL_CDMA_SMS_PRIORITY_MAX32);
  WMSTS_VERIFY( sizeof (wms_priority_e_type) == sizeof (RIL_CDMA_SMS_Priority) );
  WMSTS_VERIFY (WMS_PRIVACY_NORMAL == RIL_CDMA_SMS_PRIVACY_NORMAL);
  WMSTS_VERIFY (WMS_PRIVACY_RESTRICTED == RIL_CDMA_SMS_PRIVACY_RESTRICTED);
  WMSTS_VERIFY (WMS_PRIVACY_CONFIDENTIAL == RIL_CDMA_SMS_PRIVACY_CONFIDENTIAL);
  WMSTS_VERIFY (WMS_PRIVACY_SECRET == RIL_CDMA_SMS_PRIVACY_SECRET);
  WMSTS_VERIFY (WMS_PRIVACY_MAX32 == RIL_CDMA_SMS_PRIVACY_MAX32);
  WMSTS_VERIFY( sizeof (wms_privacy_e_type) == sizeof (RIL_CDMA_SMS_Privacy) );
  WMSTS_VERIFY( sizeof (wms_reply_option_s_type) == sizeof (RIL_CDMA_SMS_ReplyOption) );
  WMSTS_VERIFY (WMS_ALERT_MODE_DEFAULT == RIL_CDMA_SMS_ALERT_MODE_DEFAULT);
  WMSTS_VERIFY (WMS_ALERT_MODE_LOW_PRIORITY == RIL_CDMA_SMS_ALERT_MODE_LOW_PRIORITY);
  WMSTS_VERIFY (WMS_ALERT_MODE_MEDIUM_PRIORITY == RIL_CDMA_SMS_ALERT_MODE_MEDIUM_PRIORITY);
  WMSTS_VERIFY (WMS_ALERT_MODE_HIGH_PRIORITY == RIL_CDMA_SMS_ALERT_MODE_HIGH_PRIORITY);
  WMSTS_VERIFY (WMS_ALERT_MODE_OFF == RIL_CDMA_SMS_ALERT_MODE_OFF);
  WMSTS_VERIFY (WMS_ALERT_MODE_ON == RIL_CDMA_SMS_ALERT_MODE_ON);
  WMSTS_VERIFY( sizeof (wms_alert_mode_e_type) == sizeof (RIL_CDMA_SMS_AlertMode) );
  WMSTS_VERIFY (WMS_LANGUAGE_UNSPECIFIED == RIL_CDMA_SMS_LANGUAGE_UNSPECIFIED);
  WMSTS_VERIFY (WMS_LANGUAGE_ENGLISH == RIL_CDMA_SMS_LANGUAGE_ENGLISH);
  WMSTS_VERIFY (WMS_LANGUAGE_FRENCH == RIL_CDMA_SMS_LANGUAGE_FRENCH);
  WMSTS_VERIFY (WMS_LANGUAGE_SPANISH == RIL_CDMA_SMS_LANGUAGE_SPANISH);
  WMSTS_VERIFY (WMS_LANGUAGE_JAPANESE == RIL_CDMA_SMS_LANGUAGE_JAPANESE);
  WMSTS_VERIFY (WMS_LANGUAGE_KOREAN == RIL_CDMA_SMS_LANGUAGE_KOREAN);
  WMSTS_VERIFY (WMS_LANGUAGE_CHINESE == RIL_CDMA_SMS_LANGUAGE_CHINESE);
  WMSTS_VERIFY (WMS_LANGUAGE_HEBREW == RIL_CDMA_SMS_LANGUAGE_HEBREW);
  WMSTS_VERIFY (WMS_LANGUAGE_MAX32 == RIL_CDMA_SMS_LANGUAGE_MAX32);
  WMSTS_VERIFY( sizeof (wms_language_e_type) == sizeof (RIL_CDMA_SMS_Language) );
  WMSTS_VERIFY (WMS_DIGIT_MODE_4_BIT == RIL_CDMA_SMS_DIGIT_MODE_4_BIT);
  WMSTS_VERIFY (WMS_DIGIT_MODE_8_BIT == RIL_CDMA_SMS_DIGIT_MODE_8_BIT);
  WMSTS_VERIFY (WMS_DIGIT_MODE_MAX32 == RIL_CDMA_SMS_DIGIT_MODE_MAX32);
  WMSTS_VERIFY( sizeof (wms_digit_mode_e_type) == sizeof (RIL_CDMA_SMS_DigitMode) );
  WMSTS_VERIFY (WMS_NUMBER_UNKNOWN == RIL_CDMA_SMS_NUMBER_TYPE_UNKNOWN);
  WMSTS_VERIFY (WMS_NUMBER_INTERNATIONAL == RIL_CDMA_SMS_NUMBER_TYPE_INTERNATIONAL_OR_DATA_IP);
  WMSTS_VERIFY (WMS_NUMBER_NATIONAL == RIL_CDMA_SMS_NUMBER_TYPE_NATIONAL_OR_INTERNET_MAIL);
  WMSTS_VERIFY (WMS_NUMBER_NETWORK == RIL_CDMA_SMS_NUMBER_TYPE_NETWORK);
  WMSTS_VERIFY (WMS_NUMBER_SUBSCRIBER == RIL_CDMA_SMS_NUMBER_TYPE_SUBSCRIBER);
  WMSTS_VERIFY (WMS_NUMBER_ALPHANUMERIC == RIL_CDMA_SMS_NUMBER_TYPE_ALPHANUMERIC);
  WMSTS_VERIFY (WMS_NUMBER_ABBREVIATED == RIL_CDMA_SMS_NUMBER_TYPE_ABBREVIATED);
  WMSTS_VERIFY (WMS_NUMBER_RESERVED_7 == RIL_CDMA_SMS_NUMBER_TYPE_RESERVED_7);
  WMSTS_VERIFY (WMS_NUMBER_MAX32 == RIL_CDMA_SMS_NUMBER_TYPE_MAX32);
  WMSTS_VERIFY( sizeof (wms_number_type_e_type) == sizeof (RIL_CDMA_SMS_NumberType) );
  WMSTS_VERIFY (WMS_NUMBER_PLAN_UNKNOWN == RIL_CDMA_SMS_NUMBER_PLAN_UNKNOWN);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_TELEPHONY == RIL_CDMA_SMS_NUMBER_PLAN_TELEPHONY);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_2 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_2);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_DATA == RIL_CDMA_SMS_NUMBER_PLAN_DATA);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_TELEX == RIL_CDMA_SMS_NUMBER_PLAN_TELEX);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_5 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_5);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_6 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_6);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_7 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_7);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_8 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_8);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_PRIVATE == RIL_CDMA_SMS_NUMBER_PLAN_PRIVATE);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_11 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_11);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_12 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_12);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_13 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_13);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_14 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_14);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_RESERVED_15 == RIL_CDMA_SMS_NUMBER_PLAN_RESERVED_15);
  WMSTS_VERIFY (WMS_NUMBER_PLAN_MAX32 == RIL_CDMA_SMS_NUMBER_PLAN_MAX32);
  WMSTS_VERIFY( sizeof (wms_number_plan_e_type) == sizeof (RIL_CDMA_SMS_NumberPlan) );
  WMSTS_VERIFY (WMS_NUMBER_MODE_NONE_DATA_NETWORK == RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK);
  WMSTS_VERIFY (WMS_NUMBER_MODE_DATA_NETWORK == RIL_CDMA_SMS_NUMBER_MODE_DATA_NETWORK);
  WMSTS_VERIFY (WMS_NUMBER_MODE_DATA_NETWORK_MAX32 == RIL_CDMA_SMS_NUMBER_MODE_MAX32);
  WMSTS_VERIFY( sizeof (wms_number_mode_e_type) == sizeof (RIL_CDMA_SMS_NumberMode) );
  WMSTS_VERIFY( sizeof (wms_address_s_type) == sizeof (RIL_CDMA_SMS_Address) );
  WMSTS_VERIFY (WMS_DISPLAY_MODE_IMMEDIATE == RIL_CDMA_SMS_DISPLAY_MODE_IMMEDIATE);
  WMSTS_VERIFY (WMS_DISPLAY_MODE_DEFAULT == RIL_CDMA_SMS_DISPLAY_MODE_DEFAULT);
  WMSTS_VERIFY (WMS_DISPLAY_MODE_USER_INVOKE == RIL_CDMA_SMS_DISPLAY_MODE_USER_INVOKE);
  WMSTS_VERIFY (WMS_DISPLAY_MODE_RESERVED == RIL_CDMA_SMS_DISPLAY_MODE_RESERVED);
  WMSTS_VERIFY( sizeof (wms_display_mode_e_type) == sizeof (RIL_CDMA_SMS_DisplayMode) );
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_ACCEPTED == RIL_CDMA_SMS_DELIVERY_STATUS_ACCEPTED);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_DEPOSITED_TO_INTERNET == RIL_CDMA_SMS_DELIVERY_STATUS_DEPOSITED_TO_INTERNET);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_DELIVERED == RIL_CDMA_SMS_DELIVERY_STATUS_DELIVERED);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_CANCELLED == RIL_CDMA_SMS_DELIVERY_STATUS_CANCELLED);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_NETWORK_CONGESTION == RIL_CDMA_SMS_DELIVERY_STATUS_NETWORK_CONGESTION);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_NETWORK_ERROR == RIL_CDMA_SMS_DELIVERY_STATUS_NETWORK_ERROR);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_CANCEL_FAILED == RIL_CDMA_SMS_DELIVERY_STATUS_CANCEL_FAILED);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_BLOCKED_DESTINATION == RIL_CDMA_SMS_DELIVERY_STATUS_BLOCKED_DESTINATION);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_TEXT_TOO_LONG == RIL_CDMA_SMS_DELIVERY_STATUS_TEXT_TOO_LONG);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_DUPLICATE_MESSAGE == RIL_CDMA_SMS_DELIVERY_STATUS_DUPLICATE_MESSAGE);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_INVALID_DESTINATION == RIL_CDMA_SMS_DELIVERY_STATUS_INVALID_DESTINATION);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_MESSAGE_EXPIRED == RIL_CDMA_SMS_DELIVERY_STATUS_MESSAGE_EXPIRED);
  WMSTS_VERIFY (WMS_DELIVERY_STATUS_UNKNOWN_ERROR == RIL_CDMA_SMS_DELIVERY_STATUS_UNKNOWN_ERROR);
  WMSTS_VERIFY ( sizeof (wms_delivery_status_e_type) == sizeof (RIL_CDMA_SMS_DeliveryStatusE) );
  WMSTS_VERIFY ( sizeof (wms_delivery_status_s_type) == sizeof (RIL_CDMA_SMS_DeliveryStatus) );
  WMSTS_VERIFY ( sizeof (wms_other_parm_s_type) == sizeof (RIL_CDMA_SMS_OtherParm) );
  WMSTS_VERIFY (WMS_MASK_BD_NULL == RIL_CDMA_SMS_MASK_BD_NULL);
  WMSTS_VERIFY (WMS_MASK_BD_MSG_ID == RIL_CDMA_SMS_MASK_BD_MSG_ID);
  WMSTS_VERIFY (WMS_MASK_BD_USER_DATA == RIL_CDMA_SMS_MASK_BD_USER_DATA);
  WMSTS_VERIFY (WMS_MASK_BD_USER_RESP == RIL_CDMA_SMS_MASK_BD_USER_RESP);
  WMSTS_VERIFY (WMS_MASK_BD_MC_TIME == RIL_CDMA_SMS_MASK_BD_MC_TIME);
  WMSTS_VERIFY (WMS_MASK_BD_VALID_ABS == RIL_CDMA_SMS_MASK_BD_VALID_ABS);
  WMSTS_VERIFY (WMS_MASK_BD_VALID_REL == RIL_CDMA_SMS_MASK_BD_VALID_REL);
  WMSTS_VERIFY (WMS_MASK_BD_DEFER_ABS == RIL_CDMA_SMS_MASK_BD_DEFER_ABS);
  WMSTS_VERIFY (WMS_MASK_BD_DEFER_REL == RIL_CDMA_SMS_MASK_BD_DEFER_REL);
  WMSTS_VERIFY (WMS_MASK_BD_PRIORITY == RIL_CDMA_SMS_MASK_BD_PRIORITY);
  WMSTS_VERIFY (WMS_MASK_BD_PRIVACY == RIL_CDMA_SMS_MASK_BD_PRIVACY);
  WMSTS_VERIFY (WMS_MASK_BD_REPLY_OPTION == RIL_CDMA_SMS_MASK_BD_REPLY_OPTION);
  WMSTS_VERIFY (WMS_MASK_BD_NUM_OF_MSGS == RIL_CDMA_SMS_MASK_BD_NUM_OF_MSGS);
  WMSTS_VERIFY (WMS_MASK_BD_ALERT == RIL_CDMA_SMS_MASK_BD_ALERT);
  WMSTS_VERIFY (WMS_MASK_BD_LANGUAGE == RIL_CDMA_SMS_MASK_BD_LANGUAGE);
  WMSTS_VERIFY (WMS_MASK_BD_CALLBACK == RIL_CDMA_SMS_MASK_BD_CALLBACK);
  WMSTS_VERIFY (WMS_MASK_BD_DISPLAY_MODE == RIL_CDMA_SMS_MASK_BD_DISPLAY_MODE);
  WMSTS_VERIFY (WMS_MASK_BD_DEPOSIT_INDEX == RIL_CDMA_SMS_MASK_BD_DEPOSIT_INDEX);
  WMSTS_VERIFY (WMS_MASK_BD_DELIVERY_STATUS == RIL_CDMA_SMS_MASK_BD_DELIVERY_STATUS);
  WMSTS_VERIFY (WMS_MASK_BD_OTHER == RIL_CDMA_SMS_MASK_BD_OTHER);
  return 0;
}



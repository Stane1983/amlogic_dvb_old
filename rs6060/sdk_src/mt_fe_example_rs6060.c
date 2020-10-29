/****************************************************************************
* MONTAGE PROPRIETARY AND CONFIDENTIAL
* Montage Technology (Shanghai) Inc.
* All Rights Reserved
* --------------------------------------------------------------------------
*
* File:				mt_fe_example.c
*
* Current version:	0.02.00
*
* Description:		M88RS6060 IC Driver.
*
* Log:	Description				Version		Date		Author
*		---------------------------------------------------------------------
*		Initialize				0.00.00		2010-09-13	YZ.Huang
*		Added Unicable Example	0.01.00		2011-07-27	YZ.Huang
*		For RS6000				0.02.00		2013-11-04	YZ.Huang
*		For RS6060				0.02.01		2017-03-23	YZ.Huang
****************************************************************************/
#include "mt_fe_def.h"

#if 0
#define QUEUE_ID 0x8888


void os_get_mssage(OS_QUEUE QUEUE_ID, OS_MESSAGE message, U32 time_out);
void os_send_mssage(OS_QUEUE QUEUE_ID, OS_MESSAGE message);


typedef enum _MT_FE_OS_STATE
{
	OS_TIMEOUT
}MT_FE_OS_STATE;


typedef enum _MT_FE_STATE
{
	MT_FE_STATE_IDLE
	,MT_FE_STATE_CONNETED
	,MT_FE_STATE_CONNETING
	,MT_FE_STATE_NO_SIGNAL
}MT_FE_STATE;


typedef enum _MT_FE_CMD
{
	MT_FE_CMD_CONNECT
	,MT_FE_CMD_GET_CHAN_INFO
	,MT_FE_CMD_GET_SIGNAL_INFO
	,MT_FE_CMD_SLEEP
	,MT_FE_CMD_WAKE_UP
	,MT_FE_CMD_DISEQC_SET_LNB
	,MT_FE_CMD_DISEQC_SEND_TONE_BURST
	,MT_FE_CMD_DISEQC_SEND_RECEIVE_MSG
	,MT_FE_CMD_BLIND_SCAN
	,MT_FE_CMD_CANCEL_BLIND_SCAN
	,MT_FE_CMD_OPEN_UNICABLE
	,MT_FE_CMD_CLOSE_UNICABLE
	,MT_FE_CMD_DETECT_UNICABLE
}MT_FE_CMD;


typedef enum _MT_FE_UNICABLE_SATELLITE_POSITION
{
	MtFeUnicable_SAT_POS_A = 0,
	MtFeUnicable_SAT_POS_B
} MT_FE_UNICABLE_SAT_POS;


typedef enum _MT_FE_RATATE_DIRECTION
{
	MT_FE_ROTATE_WEST
	,MT_FE_ROTATE_EAST
}MT_FE_RATATE_DIRECTION;


typedef enum _MT_FE_RATATE_MODE
{
	MT_FE_ROTATE_STEP
	,MT_FE_ROTATE_CONTINOUS
}MT_FE_RATATE_MODE;
#endif

/*
	TODO:
		1. Declare a device instance first, then get it's handle.
*/
MT_FE_RS6060_DEVICE_SETTINGS rs6060_config;
MT_FE_RS6060_Device_Handle rs6060_handle = &rs6060_config;

#if 0
void main(void)
{
	OS_MESSAGE	*message;
	S8			snr, strength;
	U8			length;
	U8			data[8];
	U8			switch_port_no, horizontal_polarization, connect_time;
	U8			direction, rotate_mode, lock_timeout_cnt;
	S32			ret;
	MT_FE_STATE	state = MT_FE_STATE_IDLE;
	U32			cnt;
	U32			freq_MHz;
	U32			sym_rate_KSs;
	U32			total_packags, err_packags;
	BOOL		lnb_on_off, on_off_22k, envelop_mode;
	MT_FE_LNB_VOLTAGE	choice_13v_18v;
	MT_FE_LOCK_STATE lock_state;
	MT_FE_CHAN_INFO_DVBS2 chan_info;
	MT_FE_TYPE  dvbs_type;
	MT_FE_DiSEqC_MSG msg;
	MT_FE_DiSEqC_TONE_BURST mode;
	MT_FE_BAND_TYPE			band;
	MT_FE_LNB_LOCAL_OSC 	local_osc;
	MT_FE_UNICABLE_SAT_POS	sat_pos;
	MT_FE_BS_TP_INFO		bs;
	U32						begin_freqMHz, end_freqMHz;

	MT_FE_RET	ret = MtFeErr_Ok;


	/*alloc the memeory to store the scanned and locked TP info*/
	MT_FE_TP_INFO blindscaninfo[1000] = {0};

	void (*callback)(void);

	/*initialize the parameter*/
	bs.p_tp_info	 = blindscaninfo;
	bs.tp_max_num	 = 1000;


	connect_time = 0;


	/*
		TODO:
			2. Configurate the device instance to default.
	*/
	mt_fe_dmd_rs6060_config_default(rs6060_handle);



	/*
		TODO:
			3. If needed, users can modify the default configurations.
	*/




	/*
		TODO:
			4. Initialize the device.
	*/
	mt_fe_dmd_rs6060_init(rs6060_handle);


	while (1)
	{
		ret = os_get_mssage(QUEUE_ID, message, 100)	/*	timeout = 100ms	*/

		if (ret == OS_TIMEOUT)
		{
			/*App can use the state to show "no signal" and "connecting status" info bar.*/
			switch (state)
			{
				case MT_FE_STATE_NO_SIGNAL:
				case MT_FE_STATE_CONNETING:
					mt_fe_dmd_rs6060_get_lock_state(&lock_state);


					if (lock_state == MtFeLockState_Locked)
					{
						/* change state */
						state = MT_FE_STATE_CONNETED;

						/*
							TODO:
								send message to APP, that demodulater has Locked !

								call function 'mt_fe_dmd_get_chan_info_code_dd2k' to
								get the chan_info_code, and send it to APP.
						*/
						mt_fe_dmd_rs6060_get_channel_info(rs6060_handle, &chan_info);


						cnt = 0;
						connect_time = 0;

						break;
					}
					else if ((lock_state == MtFeLockState_Unlocked)||(lock_state == MtFeLockState_Waiting))
					{
						cnt++;
					}


					if(dvbs_type == MtFeType_DvbS)
					{
						if(sym_rate_KSs >= 2000)		lock_timeout_cnt = 10;	//1s
						else							lock_timeout_cnt = 60;	//6s
					}
					else if(dvbs_type == MtFeType_DvbS2)
					{
						if(sym_rate_KSs >= 10000)		lock_timeout_cnt = 8;  //800ms
						else if(sym_rate_KSs >= 5000)	lock_timeout_cnt = 15; //1.5s
						else if(sym_rate_KSs >= 2000)	lock_timeout_cnt = 30; //3s
						else							lock_timeout_cnt = 60; //6s
					}
					else// if(dvbs_type == MtFeType_DTV_Unknown)
					{
						if(sym_rate_KSs >= 10000)		lock_timeout_cnt = 13; //1.3s
						else if(sym_rate_KSs >= 4000)	lock_timeout_cnt = 32; //3.2s
						else if(sym_rate_KSs >= 2000)	lock_timeout_cnt = 35; //3.5s
						else							lock_timeout_cnt = 85; //8.5s
					}


					if (cnt > lock_timeout_cnt)
					{
						/*
							TODO:
								send message to APP, that demodulater Unlock !
						*/


						/*	re-connect	*/
						ret = mt_fe_dmd_rs6060_connect(rs6060_handle, freq_MHz, sym_rate_KSs, dvbs_type);

						if(ret == MtFeErr_Ok)
						{
							if(connect_time >= 1)
							{
								state = MT_FE_STATE_NO_SIGNAL;
								connect_time = 0;
							}
							else
							{
								connect_time ++;
							}
							/*	if unlock, reset cnt  */
							cnt = 0;
						}
					}
					break;

				case MT_FE_STATE_CONNETED:
					mt_fe_dmd_rs6060_get_lock_state(rs6060_handle, &lock_state);

					if (lock_state == MtFeLockState_Locked)
						cnt = 0;
					else
						cnt ++;

					if (cnt > 10)
					{
						/*
							TODO:
								send message to APP, that demodulater Drop !
						*/


						/*	re-connect	*/
						ret = mt_fe_dmd_rs6060_connect(rs6060_handle, freq_MHz, sym_rate_KSs, dvbs_type);
						if(ret == MtFeErr_Ok)
						{
							/*	change state	*/
							state = MT_FE_STATE_CONNETING;

							/*	if drop, reset chan_info_code	*/
							cnt = 0;
							connect_time ++;
						}
					}
					break;

				case MT_FE_STATE_IDLE:
				default:
					break;
			}

			continue;
		}


		switch(message)
		{
			case MT_FE_CMD_CONNECT:
				/*
				Get frequency and symbol rate and dvb_type from app.

				1.  frequency------- UNIT: MHz
				2.  symbol rate----- UNIT: KSs
				3.  dvbs_type-------- may be the three types:

						"MtFeType_DvbS"
						"MtFeType_DvbS2"
						"MtFeType_DTV_Unknown"

					If you do not know the modulator mode, you can set the dvbs_type as "MtFeType_DTV_Unknown".
				It will try DVBS and DVBS2 mode automatically.*/
				freq_MHz       = ??;
				sym_rate_KSs   = ??;
				dvbs_type	   = ??;

				ret = mt_fe_dmd_rs6060_connect(rs6060_handle, freq_MHz, sym_rate_KSs, dvbs_type);
				if(ret != MtFeErr_Ok)
				{
					// connect operation failed
					state = MT_FE_STATE_IDLE;
					cnt = 0;
					connect_time = 0;
				}
				else
				{
					state = MT_FE_STATE_CONNETING;
					cnt   = 0;

					connect_time++;
				}

				break;

			case MT_FE_CMD_BLIND_SCAN:
				/*
				Get parametres from app to do blind scan.

				1.  begin_freqMHz-------   UNIT: MHz
				2.  end_freqMHz  -------   UNIT: MHz
				3.  local_osc    -------   two types:

						"MtFeLNB_Single_Local_OSC" --> single osc
						"MtFeLNB_Dual_Local_OSC"   --> dual osc


				4.	band	     -------  two types:

						"MtFeBand_C" ---> c band
						"MtFeBand_Ku" ---> Ku band

				5.	callback     -------  function to do something(parse TS stream, eg.) after TP locked/unlocked*/


				begin_freqMHz	 = ??;
				end_freqMHz		 = ??;
				local_osc		 = ??;
				band			 = ??;
				sat_pos			 = ??;
				callback		 = ??;

				/*Detail Scan: times = 2 or more; Normal Scan: times = 1;*/
				/*Segment Lock:    algorithm = MT_FE_BS_ALGORITHM_A;
				  Whole Lock Mode: algorithm = MT_FE_BS_ALGORITHM_B;*/
				bs.bs_times 	 = 2;		//recommdated blindscan times = MT_FE_BS_TIMES = 2
				bs.bs_algorithm	 = MT_FE_BS_ALGORITHM_A;


				/*register callback function to do something(parse TS stream, eg.)*/
				mt_fe_dmd_rs6060_register_notify(callback);

				/*save bs*/
				if(rs6060_handle->lnb_cfg.bUnicable)
				{
					rs6060_handle->lnb_cfg.iBankIndex &= 0xF0;

					if(sat_pos == MtFeUnicable_SAT_POS_B)
					{
						rs6060_handle->lnb_cfg.iBankIndex |= 0x04;
					}
					else
					{
						rs6060_handle->lnb_cfg.iBankIndex &= ~0x04;
					}

					if(local_osc == MtFeLNB_Single_Local_OSC)
					{
						rs6060_handle->lnb_cfg.iBankIndex |= 0x01;				// Horizontal		18V
						rs6060_handle->lnb_cfg.iBankIndex &= ~0x02;				// Low Band			22K_OFF
						mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

						rs6060_handle->lnb_cfg.iBankIndex &= ~0x01;				// Vertical			13V
						rs6060_handle->lnb_cfg.iBankIndex &= ~0x02;				// Low Band			22K_OFF
						mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);
					}
					else
					{
						if(band == MtFeBand_Ku)
						{
							rs6060_handle->lnb_cfg.iBankIndex |= 0x01;			// Horizontal		18V
							rs6060_handle->lnb_cfg.iBankIndex &= ~0x02;			// Low Band			22K_OFF
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

							rs6060_handle->lnb_cfg.iBankIndex &= ~0x01;			// Vertical			13V
							rs6060_handle->lnb_cfg.iBankIndex &= ~0x02;			// Low Band			22K OFF
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

							rs6060_handle->lnb_cfg.iBankIndex |= 0x01;			// Horizontal		18V
							rs6060_handle->lnb_cfg.iBankIndex |= 0x02;			// High Band		22K ON
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

							rs6060_handle->lnb_cfg.iBankIndex &= ~0x01;			// Vertical			13V
							rs6060_handle->lnb_cfg.iBankIndex |= 0x02;			// High Band		22K ON
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);
						}
						else
						{
							rs6060_handle->lnb_cfg.iBankIndex |= 0x01;			// Horizontal		18V
							rs6060_handle->lnb_cfg.iBankIndex &= ~0x02;			// Low Band			22K OFF
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);
						}
					}
				}
				else
				{
					if(local_osc == MtFeLNB_Single_Local_OSC)
					{
						mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
						mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

						mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_False, MtFeLNB_13V, MtFe_False);
						mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);
					}
					else
					{
						if(band == MtFeBand_Ku)
						{
							mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

							mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_False, MtFeLNB_13V, MtFe_False);
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

							mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_True, MtFeLNB_18V, MtFe_False);
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);

							mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_True, MtFeLNB_13V, MtFe_False);
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);
						}
						else
						{
							mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
							mt_fe_dmd_rs6060_blindscan(rs6060_handle, begin_freqMHz, end_freqMHz, &bs);
						}
					}
				}
				break;


			case MT_FE_CMD_CANCEL_BLIND_SCAN:
				/*set the flag to cancel blind scan process*/
				mt_fe_dmd_rs6060_blindscan_abort(rs6060_handle, MtFe_True);
				break;

			case MT_FE_CMD_GET_CHAN_INFO:
				/*
					TODO:
						call function 'mt_fe_dmd_rs6060_get_channel_info' to
						get the channel infomation, and send it to APP.
						pls see structure MT_FE_CHAN_INFO_DVBS2
				*/
				mt_fe_dmd_rs6060_get_channel_info(rs6060_handle, &chan_info);
				break;

			case MT_FE_CMD_GET_SIGNAL_INFO:
				/*
					TODO:
						1. call 'mt_fe_dmd_rs6060_get_snr' to get the signal quality,
						2. call 'mt_fe_dmd_rs6060_get_per' to get the error packages and total packages
							to calculate the per.
						3. call 'mt_fe_dmd_rs6060_get_strength' to get the signal strength

					You can use these APIs to send the signal infomatio to APP.
				*/

				mt_fe_dmd_rs6060_get_pure_lock(rs6060_handle, &lock_state);

				if(lock_state == MtFeLockState_Locked)
				{
					mt_fe_dmd_rs6060_get_snr(rs6060_handle, &snr);
					mt_fe_dmd_rs6060_get_strength(rs6060_handle, &strength);

					/*In DVBS2 mode, get PER value; DVBS mode gets the BER value*/
					/*per/ber = err_packags/total_packags*/
					mt_fe_dmd_rs6060_get_per(rs6060_handle, &total_packags, &err_packags);
				}
				else
				{
					snr = 0;
					strength = 0;
					err_packags = 0;		/*error bit in DVBS mode; error packegs in DVBS2 mode*/
					total_packags = 0xffff;	/*total bits in DVBS mode; total packegs in DVBS2 mode*/
				}
				break;


			case MT_FE_CMD_SLEEP:
				/*
					Sleep RS6060.
				*/
				mt_fe_dmd_rs6060_sleep(rs6060_handle);
				break;

			case MT_FE_CMD_WAKE_UP:
				/*
					Wake up RS6060 and reconnect TP.
				*/
				mt_fe_dmd_rs6060_wake_up(rs6060_handle);
				mt_fe_dmd_rs6060_connect(rs6060_handle, freq_MHz, sym_rate_KSs, dvbs_type);

				break;


			case MT_FE_CMD_DISEQC_SET_LNB:
				/*
					TODO:
						You can call API 'mt_fe_dmd_rs6060_set_LNB' to set LNB on and select the LNB voltage
					according to the designed circuit.*/

				lnb_on_off 	= MtFe_True;
				on_off_22k 	= MtFe_True;
				choice_13v_18v = MtFeLNB_18V;
				envelop_mode	= MtFe_False;


				if(rs6060_handle->lnb_cfg.bUnicable)
				{
					rs6060_handle->lnb_cfg.iBankIndex &= 0xF0;

					if(choice_13v_18v == MtFeLNB_18V)
					{
						rs6060_handle->lnb_cfg.iBankIndex |= 0x01;		// Horizontal	18V
					}
					else
					{
						rs6060_handle->lnb_cfg.iBankIndex &= ~0x01;		// Vertical		13V
					}

					if(on_off_22k == MtFe_True)
					{
						rs6060_handle->lnb_cfg.iBankIndex |= 0x02;		// High Band 	22K ON
					}
					else
					{
						rs6060_handle->lnb_cfg.iBankIndex &= ~0x02;		// Low Band		22K OFF
					}

					if(sat_pos == MtFeUnicable_SAT_POS_B)
					{
						rs6060_handle->lnb_cfg.iBankIndex |= 0x04;		// Satellite Position B
					}
					else
					{
						rs6060_handle->lnb_cfg.iBankIndex &= ~0x04;		// Satellite Position A
					}
				}
				else
				{
					mt_fe_dmd_rs6060_set_LNB(rs6060_handle, lnb_on_off, on_off_22k, choice_13v_18v, envelop_mode);
				}

				break;


			case MT_FE_CMD_DISEQC_SEND_TONE_BURST:

				mode = MtFeDiSEqCToneBurst_Moulated;
				envelop_mode = MtFe_False;

				mt_fe_dmd_rs6060_DiSEqC_send_tone_burst(rs6060_handle, mode, envelop_mode);
				break;


			case MT_FE_CMD_DISEQC_SEND_RECEIVE_MSG:

				/* 1. DiSEqC switch control application example */

				switch_port_no 			= ??; 	/*switch port*/
				horizontal_polarization = ??; 	/*polarization voltage*/
				on_off_22k				= ??;   /*22k signal on/off*/
				envelop_mode			= ??;


				/* STEP1: LNB on, 22K off, 18V/13V */
				mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_False, MtFeLNB_18V/MtFeLNB_13V, MtFe_False/MtFe_True);

				/* STEP2: Send DiSEqC Message */
				/*maybe the one or more message byte(s) should be modified with some DiSEqC switches*/
				msg.data_send[0] = 0xE0; /*Framing Byte*/
				msg.data_send[1] = 0x10; /*Address Byte*/
				msg.data_send[2] = 0x38; /*Command Byte*/
				msg.data_send[3] = 0xF0 + (switch_port_no - 1) * 4; /*Data Byte*/
				msg.size_send = 4;

				if(horizontal_polarization == MtFeLNB_18V)  msg.data_send[3] |= 0x02; /*Bit 1 set 1*/
				if(on_off_22k == MtFe_True)					msg.data_send[3] |= 0x01; /*Bit 0 set 1*/
				/*only send data to device*/
				msg.is_enable_receive = FALSE;
				msg.is_envelop_mode = FALSE;
				mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(rs6060_handle, &msg);


				/*STEP3: LNB on, 22K on, 18V/13V, Maybe this step is not necessary if needn't set 22K signal on*/
				/*LNB on, 22K on, 18V/13V*/
				mt_fe_dmd_rs6060_set_LNB(rs6060_handle, MtFe_True, MtFe_True, MtFeLNB_18V/MtFeLNB_13V, MtFe_False/MtFe_True);

				/*STEP4: Connect the TP*/
				mt_fe_dmd_rs6060_connect(rs6060_handle, freq_MHz, sym_rate_KSs, dvbs_type);






				/* 2. DiSEqC Motor Rotation control application*/

				direction 	= MT_FE_ROTATE_EAST ? 0x68 : 0x69; /*rotate direction*/
				rotate_mode	= MT_FE_ROTATE_STEP ? 0xFF : 0x00; /*rotate mode*/

				/*2.1 Motor Rotation Start*/
				/* maybe the one or more message byte(s) should be modified with some DiSEqC motors*/
				msg.data_send[0] = 0xE0; 		/*Framing Byte*/
				msg.data_send[1] = 0x31; 		/*Address Byte*/
				msg.data_send[2] = direction; 	/*Command Byte1*/
				msg.data_send[3] = rotate_mode; /*Command Byte2*/
				msg.size_send = 4;

				/*only send data to device*/
				msg.is_enable_receive = FALSE;
				msg.is_envelop_mode = FALSE;

				mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(rs6060_handle, &msg);


				/*2.2 Motor Rotation Stop*/
				/* maybe the one or more message byte(s) should be modified with some DiSEqC motors*/
				msg.data_send[0] = 0xE0; 		/*Framing Byte*/
				msg.data_send[1] = 0x31; 		/*Address Byte*/
				msg.data_send[2] = 0x60; 		/*Command Byte*/
				msg.size_send = 3;

				/*only send data to device*/
				msg.is_enable_receive = FALSE;
				msg.is_envelop_mode = FALSE;

				mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(rs6060_handle, &msg);


				break;


			case MT_FE_CMD_OPEN_UNICABLE:
				rs6060_handle->lnb_cfg.bUnicable = TRUE;

				rs6060_handle->lnb_cfg.iUBIndex = ?;		// select user band index
				rs6060_handle->lnb_cfg.iUBFreqMHz = ????;	// set the user band frequency

				rs6060_handle->lnb_cfg.iBankIndex = ?;		// select bank index

				break;

			case MT_FE_CMD_CLOSE_UNICABLE:
				rs6060_handle->lnb_cfg.bUnicable = FALSE;
				break;

			default:
				break;
		}
	}
}
#endif

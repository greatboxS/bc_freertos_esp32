#ifndef root_it_handle_h
#define root_it_handle_h

#include "global_scope.h"
#include "root_nextion_callback.h"

// ethernet received data callback
void ethernet_handle(EthernetClient &client);
// nextion received data callback
void PAGE_LOADING_EVENT_CALLBACK(uint8_t _pageId, uint8_t _componentId, uint8_t _eventType);

//---------------------------------------------------------------------------------------------------
void ethernet_handle(EthernetClient &client)
{
    int receivedBytes = client.available();
    memset(httpHeader.buf, 0, sizeof(httpHeader.buf));
    client.readBytesUntil('\r\n', httpHeader.buf, receivedBytes);

    bool responsed_failed = false;

    xEventGroupClearBits(EventGroupHandle, EVENT_REQUEST_OK);

    if (strstr(httpHeader.buf, "200 OK") != nullptr)
    {
        printf("Http header: %s\r\n", httpHeader.buf);

        responsed_failed = false;

        if (CurrentRequest != nullptr)
            CurrentRequest->request_ok = true;
            
        RootNextion.GotoPage(BKanban.CurrentWindowId);
    }

    if (strstr(httpHeader.buf, "Access denied") != nullptr)
    {
        RootNextion.GotoPage(BKanban.CurrentWindowId);
        return;
    }

    if (strstr(httpHeader.buf, "400 Bad Request") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("400 Bad Request", 1000);
    }

    if (strstr(httpHeader.buf, "401 Unauthorized") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("401 Unauthorized", 1000);
    }

    if (strstr(httpHeader.buf, "404 Not Found") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("404 Not Found", 1000);
    }

    if (strstr(httpHeader.buf, "408 Request Timeout") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("408 Request Timeout", 1000);
    }

    if (strstr(httpHeader.buf, "501 Not Implemented") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("501 Not Implemented", 1000);
    }

    if (strstr(httpHeader.buf, "502 Bad Gateway") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("502 Bad Gateway", 1000);
    }

    if (strstr(httpHeader.buf, "503 Service Unavailable") != nullptr)
    {
        responsed_failed = true;
        RootNextion.showMessage("503 Service Unavailable", 1000);
    }

    if (responsed_failed)
    {
        RootNextion.GotoPage(BKanban.CurrentWindowId);
        if (CurrentRequest != nullptr)
            CurrentRequest->Cancel = true;
        return;
    }

    if (strstr(httpHeader.buf, "CUT_TIME_RECORD") != nullptr)
    {
        if (BKanban.Cutting.SubmitCutTime >= BKanban.Cutting.OldSubmitCutTime)
            BKanban.Cutting.SubmitCutTime -= BKanban.Cutting.OldSubmitCutTime;
        else
        {
            BKanban.Cutting.SubmitCutTime = 0;
            BKanban.Cutting.OldSubmitCutTime = 0;
        }

        BKanban.EthernetRespType = CUT_TIME_RECORD;
    }

    if (strstr(httpHeader.buf, "{") != nullptr)
    {
        // Get the responsed data from server
        DynamicJsonDocument JsonDoc = DynamicJsonDocument(1024 * 3);
        DeserializationError JsonErr = deserializeJson(JsonDoc, httpHeader.buf);

        printf("Json error: %s\r\n", JsonErr.c_str());

        if (!JsonErr)
        {
            JsonObject Exception = JsonDoc.getMember("Exception");
            String eop = Exception.getMember("eop").as<const char *>();
            bool Error = Exception.getMember("Error").as<bool>();
            String Message = Exception.getMember("Message").as<const char *>();

            BKanban.EthernetRespType = RESP_ERROR;

            if (Error)
            {
                Flag.RespError = true;
                if (Message.c_str() != NULL)
                {
                    Message.toLowerCase();
                    if (Message.indexOf("can not find binterface") > -1)
                    {
                        xEventGroupSetBits(EventGroupHandle, EVENT_GET_LAST_CUT_OK);
                    }
                    RootNextion.showMessage(Message.c_str());
                }
                return;
            }

            if (eop == "GET_TIME")
            {
                BKanban.EthernetRespType = RESP_GET_TIME;
                BKanban.Json_UpdateTime(JsonDoc);
                Nextion_UpdateTime();
                Ethernet_GetLastCut();
            }

            if (eop == "BEAM_GET_SCHEDULE")
            {
                BKanban.EthernetRespType = RESP_GET_SCHEDULE;
                BKanban.Cutting.Json_UpdateScheduleList(JsonDoc);
            }

            if (eop == "BEAM_GET_PO_INFO")
            {
                BKanban.EthernetRespType = RESP_GET_SCHEDULE_INFO;
                BKanban.Cutting.Json_UpdateScheduleInfo(JsonDoc);
                delay(100);
                if (BKanban.Cutting.Continue)
                {
                    Ethernet_GetSize(BKanban.BInterface);
                }
                else
                    RootNextion.GotoPage(INFO_PAGE);
            }

            if (eop == "GET_USER_INFO")
            {
                BKanban.EthernetRespType = RESP_GET_USER;
                BKanban.Cutting.Worker.UpdateUser(JsonDoc);
                Nextion_UpdateUserPage();

                if (Flag.is_adding_user())
                {
                    RootNextion.showMessage("Add new user success", 2000);
                }
                else
                {
                    RootNextion.GotoPage(LOGIN_PAGE);
                    eeprom_write_user_info(BKanban.Cutting.Worker, EPPROM_USER_INFO_ADDR);
                }
            }

            if (eop == "BEAM_GET_SIZE")
            {
                BKanban.EthernetRespType = RESP_GET_SIZE;
                BKanban.Cutting.Json_UpdateSizeList(JsonDoc);

                if (BKanban.Cutting.Continue)
                {
                    Ethernet_StartCutting(BKanban.BInterface);
                }
                RootNextion.GotoPage(CUTTING_PAGE);
            }

            if (eop == "GET_DEVICE_INFO")
            {
                BKanban.EthernetRespType = RESP_GET_MACHINE;
                BKanban.Machine.Json_UpdateMachineInfo(JsonDoc);

                eeprom_write_machine_info(BKanban.Machine, EEPROM_DEVICE_INFO_ADDR);

                RootNextion.SetPage_stringValue(MACHINE_PAGE, RootNextion.SettingMachinePageHandle.MACHINE_NAME, BKanban.Machine.MachineName);
                RootNextion.SetPage_stringValue(MACHINE_PAGE, RootNextion.SettingMachinePageHandle.MACHINE_CODE, BKanban.Machine.MachineCode);

                RootNextion.GotoPage(MACHINE_PAGE);
            }

            if (eop == "BEAM_GET_COMPONENT")
            {
                BKanban.EthernetRespType = RESP_GET_COMPONENT;
                BKanban.Cutting.Json_UpdateComponentList(JsonDoc);

                if (BKanban.Cutting.Continue)
                {
                    BKanban.SelectedComponent = BKanban.Cutting.AutoSelectCurrentComponent((int)BKanban.BInterface.Component_Id);
                    if (BKanban.SelectedComponent == NULL)
                    {
                        Ethernet_GetScheduleInfo(BKanban.BInterface);
                    }
                    else
                    {
                        Ethernet_GetScheduleInfo();
                    }
                }
                RootNextion.GotoPage(COMPONENT_PAGE);
            }

            if (eop == "BEAM_START_CUTTING")
            {
                BKanban.EthernetRespType = RESP_START_CUTTING;
                RootNextion.showMessage("Start success");
                BKanban.Json_UpdateStartCutting(JsonDoc);
                eeprom_write_binterface_id(BKanban.BInterface.BinterfaceId, EEPROM_CUT_ADDR);
                Output_Alarm(2);
                BKanban.Cutting.Continue = false;
            }

            if (eop == "BEAM_CONFIRM_SIZE")
            {
                BKanban.EthernetRespType = RESP_CONFIRM_SIZE;
                BKanban.Json_UpdateConfirmSize(JsonDoc);
                Output_Alarm();
                if (BKanban.StopCutting)
                    Ethernet_StopCutting();
            }

            if (eop == "BEAM_STOP_CUTTING")
            {
                BKanban.EthernetRespType = RESP_STOP_CUTTING;
                RootNextion.showMessage("Stop success");
                BKanban.Json_UpdateStopCutting(JsonDoc);
                Output_Alarm(3);
                Ethernet_GetLastCut();
            }

            if (eop == "BEAM_GET_LAST_CUT")
            {
                xEventGroupSetBits(EventGroupHandle, EVENT_GET_LAST_CUT_OK);
                BKanban.EthernetRespType = RESP_GET_LAST_CUT;
                BKanban.BInterface.Json_UpdateLastCut(JsonDoc);

                if (BKanban.SelectedSchedule == NULL)
                {
                    SelectedSchedule.id = BKanban.BInterface.Schedule_Id;
                    SelectedSchedule.OriginalPoId = BKanban.BInterface.OriginalPo_Id;
                    BKanban.SelectedSchedule = &SelectedSchedule;
                }

                RootNextion.SetPage_numberValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.START_SEQ, BKanban.BInterface.StartSeq);
                RootNextion.SetPage_numberValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.STOP_SEQ, BKanban.BInterface.StopSeq);
                if (BKanban.BInterface.Finish == false && BKanban.BInterface.BinterfaceId != 0 && !BKanban.Cutting.Continue)
                    ButContinueClickCallback();

                RootNextion.GotoPage(BKanban.CurrentWindowId);
            }

            if (eop == "CUT_TIME_RECORD")
            {
                BKanban.EthernetRespType = CUT_TIME_RECORD;
                BKanban.Json_UpdateSubmitCutTime(JsonDoc);
                RootNextion.SetPage_numberValue(CUTTING_PAGE, RootNextion.CuttingPageHandle.TOTAL_TIME, BKanban.Cutting.TotalCutTimes);
            }
            if (BKanban.EthernetRespType == RESP_ERROR)
                RootNextion.showMessage("Error occured while requesting to server");
        }
    }
}

//---------------------------------------------------------------------------------------------------
void PAGE_LOADING_EVENT_CALLBACK(uint8_t _pageId, uint8_t _componentId, uint8_t _eventType)
{
    f_log();
    printf("Page Id: %d, Component Id: %d, Event Type: %d\r\n", _pageId, _componentId, _eventType);

    switch (_componentId)
    {
    case 200:
        //New page loading
        BKanban.NewPageLoading = true;
        BKanban.CurrentPageId = _pageId;
        break;

    default: // New button event
        switch (_pageId)
        {
        case HIS_PAGE:
            if (_componentId == RootNextion.HisPageHandle.BUT_CONTINUE_ID)
                ButContinueClickCallback();
            if (_componentId == RootNextion.HisPageHandle.BUT_GOT_MACHINE_ID)
            {
                ButSettingMachineClickCallback();
            }
            if (_componentId == RootNextion.HisPageHandle.BUT_GO_SEARCH_ID)
                ButGoSearchClickCallBack();
            if (_componentId == RootNextion.HisPageHandle.BUT_GO_USER_ID)
                ButSettingUserClickCallback();
            break;

        case SEARCH_PAGE:
            if (_componentId == RootNextion.SearchPageHandle.BUT_SEARCHING_ID)
                ButSearchingClickCallback();
            if (_componentId >= RootNextion.SearchPageHandle.FIRST_SELECT_PO_BUT_ID && _componentId < RootNextion.SearchPageHandle.LAST_SELECT_PO_BUT_ID)
                ButSelectScheduleCallback(_componentId);
            if (_componentId == RootNextion.SearchPageHandle.GOT_BACK)
            {
                ButSearchGobackClickCallback();
            }

            break;

        case INFO_PAGE:
            if (_componentId == RootNextion.InfoPageHandle.BUT_NEXT_ID)
                ButInfoGoNextClickCallback();
            if (_componentId == RootNextion.InfoPageHandle.BUT_BACK_ID)
                ButInfoGoBackClickCallback();
            break;

        case CUTTING_PAGE:
            if (_componentId == RootNextion.CuttingPageHandle.BUT_START_ID)
                ButStartCuttingClickCallback();
            if (_componentId == RootNextion.CuttingPageHandle.BUT_GET_SIZE)
                ButGetSizeClickCallback();
            if (_componentId == RootNextion.CuttingPageHandle.BUT_STOP_ID)
                ButStopCuttingClickCallback();
            if (_componentId >= RootNextion.CuttingPageHandle.FIRST_SELECT_SIZE_BUT_ID && _componentId < RootNextion.CuttingPageHandle.LAST_SIZE)
                ButSelectSizeClickCallback(_componentId);
            if (_componentId == RootNextion.CuttingPageHandle.BUT_GO_BACK)
                ButCuttingBackClickCallback();
            break;

        case CONFIRM_SIZE_PAGE:
            if (_componentId == RootNextion.ConfirmSizeHandle.BUT_FINISH_ID)
                ButConfirmFinishSize();
            if (_componentId == RootNextion.ConfirmSizeHandle.BUT_NOT_YET_ID)
                ButConfirmNotYetFinishSize();
            break;

        case MACHINE_PAGE:
            if (_componentId == RootNextion.SettingMachinePageHandle.BUT_SAVE_SERVER_INFO_ID)
                ButSaveServerClickCallback();
            if (_componentId == RootNextion.SettingMachinePageHandle.BUT_SAVE_MACHINE_INFO_ID)
                ButSaveMachineClickCallback();
            if (_componentId == RootNextion.SettingMachinePageHandle.BUT_RESET_ID)
                ButResetClickCallback();
            if (_componentId == RootNextion.SettingMachinePageHandle.BUT_SAVE_MAC)
                ButSaveMacClickCallback();
            break;

        case NEW_USER_PAGE:
            if (_componentId == RootNextion.NewUserHandle.BUT_GO_BACK_ID)
                RootNextion.GotoPage(HIS_PAGE);

            if (_componentId == RootNextion.NewUserHandle.BUT_ADD_NEW_ID)
            {
                ButAddNewUserClickCallback();
                Flag.IsAddNewUser = true;
            }

            break;

        case MESSAGE_PAGE:
            if (_componentId == 100)
            {
                WaitingTimeOutCallback();
            }
            break;

        case COMPONENT_PAGE:
            if (_componentId >= RootNextion.ComponentHandle.FIST_SELECT_COMPONENT_ID &&
                _componentId < RootNextion.ComponentHandle.LAST_COMPONENT)
                ButSelectComponentClickCallback(_componentId);

            if (_componentId == RootNextion.ComponentHandle.GO_BACK)
                ButGobackClickCallback();
            break;

        case WAITING_PAGE:
            if (_componentId == 100)
                WaitingTimeOutCallback();
            break;

        case LOGIN_PAGE:
            if (_componentId == RootNextion.LoginPageHandle.BUT_GO_BACK_ID)
                WaitingTimeOutCallback();
            break;
        }
        break;
    }
}
#endif
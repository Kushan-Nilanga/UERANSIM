//
// This file is a part of UERANSIM open source project.
// Copyright (c) 2021 ALİ GÜNGÖR.
//
// The software and all associated files are licensed under GPL-3.0
// and subject to the terms and conditions defined in LICENSE file.
//

#include "cmd_handler.hpp"

#include <ue/app/task.hpp>
#include <ue/nas/task.hpp>
#include <ue/rls/task.hpp>
#include <ue/rrc/task.hpp>
#include <ue/tun/task.hpp>
#include <utils/common.hpp>
#include <utils/printer.hpp>

#define PAUSE_CONFIRM_TIMEOUT 3000
#define PAUSE_POLLING 10

// todo add coverage again to cli
static std::string SignalDescription(int dbm)
{
    if (dbm > -90)
        return "Excellent";
    if (dbm > -105)
        return "Good";
    if (dbm > -120)
        return "Fair";
    return "Poor";
}

namespace nr::ue
{

void UeCmdHandler::sendResult(const InetAddress &address, const std::string &output)
{
    m_base->cliCallbackTask->push(new app::NwCliSendResponse(address, output, false));
}

void UeCmdHandler::sendError(const InetAddress &address, const std::string &output)
{
    m_base->cliCallbackTask->push(new app::NwCliSendResponse(address, output, true));
}

void UeCmdHandler::pauseTasks()
{
    m_base->nasTask->requestPause();
    m_base->rrcTask->requestPause();
    m_base->rlsTask->requestPause();
}

void UeCmdHandler::unpauseTasks()
{
    m_base->nasTask->requestUnpause();
    m_base->rrcTask->requestUnpause();
    m_base->rlsTask->requestUnpause();
}

bool UeCmdHandler::isAllPaused()
{
    if (!m_base->nasTask->isPauseConfirmed())
        return false;
    if (!m_base->rrcTask->isPauseConfirmed())
        return false;
    if (!m_base->rlsTask->isPauseConfirmed())
        return false;
    return true;
}

void UeCmdHandler::handleCmd(NmUeCliCommand &msg)
{
    pauseTasks();

    uint64_t currentTime = utils::CurrentTimeMillis();
    uint64_t endTime = currentTime + PAUSE_CONFIRM_TIMEOUT;

    bool isPaused = false;
    while (currentTime < endTime)
    {
        currentTime = utils::CurrentTimeMillis();
        if (isAllPaused())
        {
            isPaused = true;
            break;
        }
        utils::Sleep(PAUSE_POLLING);
    }

    if (!isPaused)
    {
        sendError(msg.address, "UE is unable process command due to pausing timeout");
    }
    else
    {
        handleCmdImpl(msg);
    }

    unpauseTasks();
}

void UeCmdHandler::handleCmdImpl(NmUeCliCommand &msg)
{
    switch (msg.cmd->present)
    {
    case app::UeCliCommand::STATUS: {
        Json json = Json::Obj({
            {"cm-state", ToJson(m_base->nasTask->mm->m_cmState)},
            {"rm-state", ToJson(m_base->nasTask->mm->m_rmState)},
            {"mm-state", ToJson(m_base->nasTask->mm->m_mmSubState)},
            {"5u-state", ToJson(m_base->nasTask->mm->m_storage->uState->get())},
            {"sim-inserted", m_base->nasTask->mm->m_usim->isValid()},
            {"stored-suci", ToJson(m_base->nasTask->mm->m_storage->storedSuci->get())},
            {"stored-guti", ToJson(m_base->nasTask->mm->m_storage->storedGuti->get())},
            {"has-emergency", ::ToJson(m_base->nasTask->mm->hasEmergency())},
        });
        sendResult(msg.address, json.dumpYaml());
        break;
    }
    case app::UeCliCommand::INFO: {
        sendResult(msg.address, ToJson(*m_base->config).dumpYaml());
        break;
    }
    case app::UeCliCommand::TIMERS: {
        sendResult(msg.address, ToJson(m_base->nasTask->timers).dumpYaml());
        break;
    }
    case app::UeCliCommand::DE_REGISTER: {
        m_base->nasTask->mm->deregistrationRequired(msg.cmd->deregCause);

        if (msg.cmd->deregCause != EDeregCause::SWITCH_OFF)
            sendResult(msg.address, "De-registration procedure triggered");
        else
            sendResult(msg.address, "De-registration procedure triggered. UE device will be switched off.");
        break;
    }
    case app::UeCliCommand::PS_RELEASE: {
        for (int i = 0; i < msg.cmd->psCount; i++)
            m_base->nasTask->sm->sendReleaseRequest(static_cast<int>(msg.cmd->psIds[i]) % 16);
        sendResult(msg.address, "PDU session release procedure(s) triggered");
        break;
    }
    case app::UeCliCommand::PS_RELEASE_ALL: {
        m_base->nasTask->sm->sendReleaseRequestForAll();
        sendResult(msg.address, "PDU session release procedure(s) triggered");
        break;
    }
    case app::UeCliCommand::PS_ESTABLISH: {
        SessionConfig config;
        config.type = nas::EPduSessionType::IPV4;
        config.isEmergency = msg.cmd->isEmergency;
        config.apn = msg.cmd->apn;
        config.sNssai = msg.cmd->sNssai;
        m_base->nasTask->sm->sendEstablishmentRequest(config);
        sendResult(msg.address, "PDU session establishment procedure triggered");
        break;
    }
    case app::UeCliCommand::PS_LIST: {
        Json json = Json::Obj({});
        for (auto *pduSession : m_base->nasTask->sm->m_pduSessions)
        {
            if (pduSession->psi == 0 || pduSession->psState == EPsState::INACTIVE)
                continue;

            auto obj = Json::Obj({
                {"state", ToJson(pduSession->psState)},
                {"session-type", ToJson(pduSession->sessionType)},
                {"apn", ::ToJson(pduSession->apn)},
                {"s-nssai", ToJson(pduSession->sNssai)},
                {"emergency", pduSession->isEmergency},
                {"address", ::ToJson(pduSession->pduAddress)},
                {"ambr", ::ToJson(pduSession->sessionAmbr)},
                {"data-pending", pduSession->uplinkPending},
            });

            json.put("PDU Session" + std::to_string(pduSession->psi), obj);
        }
        sendResult(msg.address, json.dumpYaml());
        break;
    }
    }
}

} // namespace nr::ue

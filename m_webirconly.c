/*
* Este módulo permite colocar en modo +W un canal y así permitir enviar texto a los usuarios conectados por webirc. 
* Es necesario el módulo m_showwebirc creado por k4be. 
*
* This module allows to place in +W mode a channel and thus allow to send text to the users connected by webirc.
* The m_showwebirc module created by k4be is required
*/
 
#include "unrealircd.h"
 
ModuleHeader MOD_HEADER(m_webirconly)
  = {
    "chanmodes/webirconly",
    "$Id: v0.01 2018/12/21 Mauropek$",
    "Channel Mode +W",
    "3.2-b8-1",
    NULL
    };
 
Cmode_t EXTCMODE_WEBIRCONLY;
 
#define Iswebirconly(chptr)    (chptr->mode.extmode & EXTCMODE_WEBIRCONLY)
 
DLLFUNC int webirconly_can_send (aClient* cptr, aChannel *chptr, char* message, Membership* lp, int notice);
DLLFUNC char *webirconly_part_message (aClient* sptr, aChannel *chptr, char* comment);
 
static int is_webirc (aClient *cptr);
 
MOD_TEST(m_webirconly)
{
    return MOD_SUCCESS;
}
 
MOD_INIT(m_webirconly)
{
CmodeInfo req;
    memset(&req, 0, sizeof(req));
    req.paracount = 0;
    req.flag = 'W';
    req.is_ok = extcmode_default_requirehalfop;
    CmodeAdd(modinfo->handle, req, &EXTCMODE_WEBIRCONLY);
   
    HookAdd(modinfo->handle, HOOKTYPE_CAN_SEND, 0, webirconly_can_send);
    HookAddPChar(modinfo->handle, HOOKTYPE_PRE_LOCAL_PART, 0, webirconly_part_message);
 
   return MOD_SUCCESS;
}
 
MOD_LOAD(m_webirconly)
{
    return MOD_SUCCESS;
}
 
MOD_UNLOAD(m_webirconly)
{
    return MOD_SUCCESS;
}
 
DLLFUNC char *webirconly_part_message (aClient *sptr, aChannel *chptr, char *comment)
{
    if (!comment)
        return NULL;
 
    if (Iswebirconly(chptr) && !is_webirc(sptr))
        return NULL;
 
    return comment;
}
 
DLLFUNC int webirconly_can_send (aClient *cptr, aChannel *chptr, char *message, Membership *lp, int notice)
{
    Hook *h;
    int i;
 
    if (Iswebirconly(chptr) && !is_webirc(cptr) &&
            (!lp
            || !(lp->flags & (CHFL_CHANOP | CHFL_VOICE | CHFL_CHANOWNER |
            CHFL_HALFOP | CHFL_CHANPROT))))
    {
        for (h = Hooks[HOOKTYPE_CAN_BYPASS_CHANNEL_MESSAGE_RESTRICTION]; h; h = h->next)
        {
            i = (*(h->func.intfunc))(cptr, chptr, BYPASS_CHANMSG_MODERATED);
            if (i != HOOK_CONTINUE)
                break;
        }
        if (i == HOOK_ALLOW)
            return HOOK_CONTINUE; /* bypass +W restriction */
 
        return CANNOT_SEND_MODREG; /* BLOCK message */
    }
 
    return HOOK_CONTINUE;
}
 
static int is_webirc (aClient *cptr){
    ModDataInfo *moddata;
    moddata = findmoddata_byname("webirc", MODDATATYPE_CLIENT);
    if(moddata == NULL) return 0;
    if(moddata_client(cptr, moddata).l){
        return 1;
    }
    return 0;
}

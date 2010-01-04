/* Unreal IRCD 3.2.x functions
 *
 * (C) 2003-2009 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 *
 */

/*************************************************************************/

#include "services.h"
#include "pseudo.h"

IRCDVar myIrcd[] = {
	{"UnrealIRCd 3.2.x",		/* ircd name */
	 "+Soi",					/* Modes used by pseudoclients */
	 5,						 /* Chan Max Symbols	 */
	 "+ao",					 /* Channel Umode used by Botserv bots */
	 1,						 /* SVSNICK */
	 1,						 /* Vhost  */
	 1,						 /* Supports SGlines	 */
	 1,						 /* Supports SQlines	 */
	 1,						 /* Supports SZlines	 */
	 3,						 /* Number of server args */
	 0,						 /* Join 2 Set		   */
	 0,						 /* Join 2 Message	   */
	 1,						 /* TS Topic Forward	 */
	 0,						 /* TS Topci Backward	*/
	 0,						 /* Chan SQlines		 */
	 0,						 /* Quit on Kill		 */
	 1,						 /* SVSMODE unban		*/
	 1,						 /* Reverse			  */
	 1,						 /* vidents			  */
	 1,						 /* svshold			  */
	 1,						 /* time stamp on mode   */
	 1,						 /* NICKIP			   */
	 1,						 /* O:LINE			   */
	 1,						 /* UMODE			   */
	 1,						 /* VHOST ON NICK		*/
	 1,						 /* Change RealName	  */
	 0,						 /* On nick change check if they could be identified */
	 1,						 /* No Knock requires +i */
	 NULL,					  /* CAPAB Chan Modes			 */
	 1,						 /* We support Unreal TOKENS */
	 1,						 /* TIME STAMPS are BASE64 */
	 '&',					   /* SJOIN ban char */
	 '\"',					  /* SJOIN except char */
	 '\'',					  /* SJOIN invite char */
	 1,						 /* Can remove User Channel Modes with SVSMODE */
	 0,						 /* Sglines are not enforced until user reconnects */
	 0,						 /* ts6 */
	 0,						 /* p10 */
	 NULL,					  /* character set */
	 0,						 /* CIDR channelbans */
	 "$",					   /* TLD Prefix for Global */
	 false,					/* Auth for users is sent after the initial NICK/UID command */
	 }
	,
	{NULL}
};

IRCDCAPAB myIrcdcap[] = {
	{
	 CAPAB_NOQUIT,			  /* NOQUIT	   */
	 0,						 /* TSMODE	   */
	 0,						 /* UNCONNECT	*/
	 CAPAB_NICKIP,			  /* NICKIP	   */
	 0,						 /* SJOIN		*/
	 CAPAB_ZIP,				 /* ZIP		  */
	 0,						 /* BURST		*/
	 0,						 /* TS5		  */
	 0,						 /* TS3		  */
	 0,						 /* DKEY		 */
	 0,						 /* PT4		  */
	 0,						 /* SCS		  */
	 0,						 /* QS		   */
	 0,						 /* UID		  */
	 0,						 /* KNOCK		*/
	 0,						 /* CLIENT	   */
	 0,						 /* IPV6		 */
	 0,						 /* SSJ5		 */
	 0,						 /* SN2		  */
	 CAPAB_TOKEN,			   /* TOKEN		*/
	 0,						 /* VHOST		*/
	 CAPAB_SSJ3,				/* SSJ3		 */
	 CAPAB_NICK2,			   /* NICK2		*/
	 CAPAB_VL,				  /* VL		   */
	 CAPAB_TLKEXT,			  /* TLKEXT	   */
	 0,						 /* DODKEY	   */
	 0,						 /* DOZIP		*/
	 CAPAB_CHANMODE,			/* CHANMODE			 */
	 CAPAB_SJB64,
	 CAPAB_NICKCHARS,
	 }
};

/* svswatch
 * parv[0] - sender
 * parv[1] - target nick
 * parv[2] - parameters
 */
void unreal_cmd_svswatch(const char *sender, const char *nick, const char *parm)
{
	send_cmd(sender, "Bw %s :%s", nick, parm);
}

void unreal_cmd_netinfo(int ac, const char **av)
{
	send_cmd(NULL, "AO %ld %ld %d %s 0 0 0 :%s", static_cast<long>(maxusercnt), static_cast<long>(time(NULL)), atoi(av[2]), av[3], av[7]);
}
/* PROTOCTL */
/*
   NICKv2 = Nick Version 2
   VHP	= Sends hidden host
   UMODE2 = sends UMODE2 on user modes
   NICKIP = Sends IP on NICK
   TOKEN  = Use tokens to talk
   SJ3	= Supports SJOIN
   NOQUIT = No Quit
   TKLEXT = Extended TKL we don't use it but best to have it
   SJB64  = Base64 encoded time stamps
   VL	 = Version Info
   NS	 = Config.Numeric Server

*/
void unreal_cmd_capab()
{
	if (Config.Numeric)
	{
		send_cmd(NULL, "PROTOCTL NICKv2 VHP UMODE2 NICKIP TOKEN SJOIN SJOIN2 SJ3 NOQUIT TKLEXT SJB64 VL");
	}
	else
	{
		send_cmd(NULL, "PROTOCTL NICKv2 VHP UMODE2 NICKIP TOKEN SJOIN SJOIN2 SJ3 NOQUIT TKLEXT SJB64");
	}
}

/* PASS */
void unreal_cmd_pass(const char *pass)
{
	send_cmd(NULL, "PASS :%s", pass);
}

/* CHGHOST */
void unreal_cmd_chghost(const char *nick, const char *vhost)
{
	if (!nick || !vhost) {
		return;
	}
	send_cmd(Config.ServerName, "AL %s %s", nick, vhost);
}

/* CHGIDENT */
void unreal_cmd_chgident(const char *nick, const char *vIdent)
{
	if (!nick || !vIdent) {
		return;
	}
	send_cmd(Config.ServerName, "AZ %s %s", nick, vIdent);
}



class UnrealIRCdProto : public IRCDProto
{
	/* SVSNOOP */
	void SendSVSNOOP(const char *server, int set)
	{
		send_cmd(NULL, "f %s %s", server, set ? "+" : "-");
	}

	void SendAkillDel(Akill *ak)
	{
		send_cmd(NULL, "BD - G %s %s %s", ak->user, ak->host, Config.s_OperServ);
	}

	void SendTopic(BotInfo *whosets, Channel *c, const char *whosetit, const char *topic)
	{
		send_cmd(whosets->nick, ") %s %s %lu :%s", c->name.c_str(), whosetit, static_cast<unsigned long>(c->topic_time), topic);
	}

	void SendVhostDel(User *u)
	{
		BotInfo *bi = findbot(Config.s_HostServ);
		u->RemoveMode(bi, UMODE_CLOAK);
		u->RemoveMode(bi, UMODE_VHOST);
		ModeManager::ProcessModes();
		u->SetMode(bi, UMODE_CLOAK);
		ModeManager::ProcessModes();
	}

	void SendAkill(Akill *ak)
	{
		// Calculate the time left before this would expire, capping it at 2 days
		time_t timeleft = ak->expires - time(NULL);
		if (timeleft > 172800) timeleft = 172800;
		send_cmd(NULL, "BD + G %s %s %s %ld %ld :%s", ak->user, ak->host, ak->by, static_cast<long>(time(NULL) + timeleft), static_cast<long>(ak->expires), ak->reason);
	}

	void SendSVSKillInternal(BotInfo *source, User *user, const char *buf)
	{
		send_cmd(source ? source->nick : Config.ServerName, "h %s :%s", user->nick.c_str(), buf);
	}

	/*
	 * m_svsmode() added by taz
	 * parv[0] - sender
	 * parv[1] - username to change mode for
	 * parv[2] - modes to change
	 * parv[3] - Service Stamp (if mode == d)
	 */
	void SendSVSMode(User *u, int ac, const char **av)
	{
		if (ac >= 1) {
			if (!u || !av[0]) return;
			this->SendModeInternal(NULL, u, merge_args(ac, av));
		}
	}

	void SendModeInternal(BotInfo *source, Channel *dest, const char *buf)
	{
		if (!buf) return;
		send_cmd(source->nick, "G %s %s", dest->name.c_str(), buf);
	}

	void SendModeInternal(BotInfo *bi, User *u, const char *buf)
	{
		if (!buf) return;
		send_cmd(bi ? bi->nick : Config.ServerName, "v %s %s", u->nick.c_str(), buf);
	}

	void SendClientIntroduction(const std::string &nick, const std::string &user, const std::string &host, const std::string &real, const char *modes, const std::string &uid)
	{
		EnforceQlinedNick(nick, Config.s_BotServ);
		send_cmd(NULL, "& %s 1 %ld %s %s %s 0 %s %s%s :%s", nick.c_str(), static_cast<long>(time(NULL)), user.c_str(), host.c_str(), Config.ServerName, modes, host.c_str(), myIrcd->nickip ? " *" : " ", real.c_str());
		SendSQLine(nick, "Reserved for services");
	}

	void SendKickInternal(BotInfo *source, Channel *chan, User *user, const char *buf)
	{
		if (buf) send_cmd(source->nick, "H %s %s :%s", chan->name.c_str(), user->nick.c_str(), buf);
		else send_cmd(source->nick, "H %s %s", chan->name.c_str(), user->nick.c_str());
	}

	void SendNoticeChanopsInternal(BotInfo *source, Channel *dest, const char *buf)
	{
		if (!buf) return;
		send_cmd(source->nick, "B @%s :%s", dest->name.c_str(), buf);
	}

	/* SERVER name hop descript */
	/* Unreal 3.2 actually sends some info about itself in the descript area */
	void SendServer(Server *server)
	{
		if (Config.Numeric)
			send_cmd(NULL, "SERVER %s %d :U0-*-%s %s", server->name, server->hops, Config.Numeric, server->desc);
		else
			send_cmd(NULL, "SERVER %s %d :%s", server->name, server->hops, server->desc);
	}

	/* JOIN */
	void SendJoin(BotInfo *user, const char *channel, time_t chantime)
	{
		send_cmd(Config.ServerName, "~ !%s %s :%s", base64enc(static_cast<long>(chantime)), channel, user->nick.c_str());
	}

	/* unsqline
	**	parv[0] = sender
	**	parv[1] = nickmask
	*/
	void SendSQLineDel(const std::string &user)
	{
		if (user.empty())
			return;
		send_cmd(NULL, "d %s", user.c_str());
	}


	/* SQLINE */
	/*
	**	parv[0] = sender
	**	parv[1] = nickmask
	**	parv[2] = reason
	**
	** - Unreal will translate this to TKL for us
	**
	*/
	void SendSQLine(const std::string &mask, const std::string &reason)
	{
		if (mask.empty() || reason.empty())
			return;
		send_cmd(NULL, "c %s :%s", mask.c_str(), reason.c_str());
	}

	/*
	** svso
	**	  parv[0] = sender prefix
	**	  parv[1] = nick
	**	  parv[2] = options
	*/
	void SendSVSO(const char *source, const char *nick, const char *flag)
	{
		if (!source || !nick || !flag) return;
		send_cmd(source, "BB %s %s", nick, flag);
	}

	/* NICK <newnick>  */
	void SendChangeBotNick(BotInfo *oldnick, const char *newnick)
	{
		if (!oldnick || !newnick) return;
		send_cmd(oldnick->nick, "& %s %ld", newnick, static_cast<long>(time(NULL)));
	}

	/* Functions that use serval cmd functions */

	void SendVhost(User *u, const char *vIdent, const char *vhost)
	{
		if (vIdent) unreal_cmd_chgident(u->nick.c_str(), vIdent);
		unreal_cmd_chghost(u->nick.c_str(), vhost);
	}

	void SendConnect()
	{
		unreal_cmd_capab();
		unreal_cmd_pass(uplink_server->password);
		if (Config.Numeric)
			me_server = new_server(NULL, Config.ServerName, Config.ServerDesc, SERVER_ISME, Config.Numeric);
		else
			me_server = new_server(NULL, Config.ServerName, Config.ServerDesc, SERVER_ISME, NULL);
		SendServer(me_server);
	}

	/* SVSHOLD - set */
	void SendSVSHold(const char *nick)
	{
		send_cmd(NULL, "BD + Q H %s %s %ld %ld :%s", nick, Config.ServerName, static_cast<long>(time(NULL) + Config.NSReleaseTimeout),
			static_cast<long>(time(NULL)), "Being held for registered user");
	}

	/* SVSHOLD - release */
	void SendSVSHoldDel(const char *nick)
	{
		send_cmd(NULL, "BD - Q * %s %s", nick, Config.ServerName);
	}

	/* UNSGLINE */
	/*
	 * SVSNLINE - :realname mask
	*/
	void SendSGLineDel(SXLine *sx)
	{
		send_cmd(NULL, "BR - :%s", sx->mask);
	}

	/* UNSZLINE */
	void SendSZLineDel(SXLine *sx)
	{
		send_cmd(NULL, "BD - Z * %s %s", sx->mask, Config.s_OperServ);
	}

	/* SZLINE */
	void SendSZLine(SXLine *sx)
	{
		send_cmd(NULL, "BD + Z * %s %s %ld %ld :%s", sx->mask, sx->by, static_cast<long>(time(NULL) + 172800), static_cast<long>(time(NULL)), sx->reason);
	}

	/* SGLINE */
	/*
	 * SVSNLINE + reason_where_is_space :realname mask with spaces
	*/
	void SendSGLine(SXLine *sx)
	{
		char edited_reason[BUFSIZE];
		strlcpy(edited_reason, sx->reason, BUFSIZE);
		strnrepl(edited_reason, BUFSIZE, " ", "_");
		send_cmd(NULL, "BR + %s :%s", edited_reason, sx->mask);
	}

	/* SVSMODE -b */
	void SendBanDel(Channel *c, const std::string &nick)
	{
		SendSVSModeChan(c, "-b", nick.empty() ? NULL : nick.c_str());
	}


	/* SVSMODE channel modes */

	void SendSVSModeChan(Channel *c, const char *mode, const char *nick)
	{
		if (nick) send_cmd(Config.ServerName, "n %s %s %s", c->name.c_str(), mode, nick);
		else send_cmd(Config.ServerName, "n %s %s", c->name.c_str(), mode);
	}

	/* svsjoin
		parv[0] - sender
		parv[1] - nick to make join
		parv[2] - channel to join
		parv[3] - (optional) channel key(s)
	*/
	/* In older Unreal SVSJOIN and SVSNLINE tokens were mixed so SVSJOIN and SVSNLINE are broken
	   when coming from a none TOKEN'd server
	*/
	void SendSVSJoin(const char *source, const char *nick, const char *chan, const char *param)
	{
		if (param) send_cmd(source, "BX %s %s :%s", nick, chan, param);
		else send_cmd(source, "BX %s :%s", nick, chan);
	}

	/* svspart
		parv[0] - sender
		parv[1] - nick to make part
		parv[2] - channel(s) to part
	*/
	void SendSVSPart(const char *source, const char *nick, const char *chan)
	{
		send_cmd(source, "BT %s :%s", nick, chan);
	}

	void SendSWhois(const char *source, const char *who, const char *mask)
	{
		send_cmd(source, "BA %s :%s", who, mask);
	}

	void SendEOB()
	{
		send_cmd(Config.ServerName, "ES");
	}

	/*
	  1 = valid nick
	  0 = nick is in valid
	*/
	int IsNickValid(const char *nick)
	{
		if (!stricmp("ircd", nick) || !stricmp("irc", nick))
			return 0;
		return 1;
	}

	int IsChannelValid(const char *chan)
	{
		if (strchr(chan, ':') || *chan != '#') return 0;
		return 1;
	}

	void SetAutoIdentificationToken(User *u)
	{
		char svidbuf[15];

		if (!u->nc)
			return;

		srand(time(NULL));
		snprintf(svidbuf, sizeof(svidbuf), "%d", rand());

		u->nc->Shrink("authenticationtoken");
		u->nc->Extend("authenticationtoken", new ExtensibleItemPointerArray<char>(sstrdup(svidbuf)));

		BotInfo *bi = findbot(Config.s_NickServ);
		u->SetMode(bi, UMODE_REGISTERED);
		ircdproto->SendMode(bi, u, "+d %s", svidbuf);
	}

	void SendUnregisteredNick(User *u)
	{
		BotInfo *bi = findbot(Config.s_NickServ);
		u->RemoveMode(bi, UMODE_REGISTERED);
		ircdproto->SendMode(bi, u, "+d 1");
	}

} ircd_proto;



/* Event: PROTOCTL */
int anope_event_capab(const char *source, int ac, const char **av)
{
	capab_parse(ac, av);
	return MOD_CONT;
}

/* Events */
int anope_event_ping(const char *source, int ac, const char **av)
{
	if (ac < 1)
		return MOD_CONT;
	ircdproto->SendPong(ac > 1 ? av[1] : Config.ServerName, av[0]);
	return MOD_CONT;
}

/* netinfo
 *  argv[0] = max global count
 *  argv[1] = time of end sync
 *  argv[2] = unreal protocol using (numeric)
 *  argv[3] = cloak-crc (> u2302)
 *  argv[4] = free(**)
 *  argv[5] = free(**)
 *  argv[6] = free(**)
 *  argv[7] = ircnet
 */
int anope_event_netinfo(const char *source, int ac, const char **av)
{
	unreal_cmd_netinfo(ac, av);
	return MOD_CONT;
}

int anope_event_eos(const char *source, int ac, const char **av)
{
	Server *s;
	s = findserver(servlist, source);
	/* If we found a server with the given source, that one just
	 * finished bursting. If there was no source, then our uplink
	 * server finished bursting. -GD
	 */
	if (!s && serv_uplink)
		s = serv_uplink;
	finish_sync(s, 1);
	return MOD_CONT;
}

int anope_event_436(const char *source, int ac, const char **av)
{
	if (ac < 1)
		return MOD_CONT;

	m_nickcoll(av[0]);
	return MOD_CONT;
}

/*
** away
**	  parv[0] = sender prefix
**	  parv[1] = away message
*/
int anope_event_away(const char *source, int ac, const char **av)
{
	if (!source) {
		return MOD_CONT;
	}
	m_away(source, (ac ? av[0] : NULL));
	return MOD_CONT;
}

/*
** m_topic
**	parv[0] = sender prefix
**	parv[1] = topic text
**
**	For servers using TS:
**	parv[0] = sender prefix
**	parv[1] = channel name
**	parv[2] = topic nickname
**	parv[3] = topic time
**	parv[4] = topic text
*/
int anope_event_topic(const char *source, int ac, const char **av)
{
	if (ac != 4)
		return MOD_CONT;
	do_topic(source, ac, av);
	return MOD_CONT;
}

int anope_event_squit(const char *source, int ac, const char **av)
{
	if (ac != 2)
		return MOD_CONT;
	do_squit(source, ac, av);
	return MOD_CONT;
}

int anope_event_quit(const char *source, int ac, const char **av)
{
	if (ac != 1)
		return MOD_CONT;
	do_quit(source, ac, av);
	return MOD_CONT;
}

int anope_event_mode(const char *source, int ac, const char **av)
{
	if (ac < 2)
		return MOD_CONT;

	if (*av[0] == '#' || *av[0] == '&') {
		do_cmode(source, ac, av);
	} else {
		do_umode(source, ac, av);
	}
	return MOD_CONT;
}

/* This is used to strip the TS from the end of the mode stirng */
int anope_event_gmode(const char *source, int ac, const char **av)
{
        if (findserver(servlist, source))
                --ac;
        return anope_event_mode(source, ac, av);
}

/* Unreal sends USER modes with this */
/*
	umode2
	parv[0] - sender
	parv[1] - modes to change
*/
int anope_event_umode2(const char *source, int ac, const char **av)
{
	if (ac < 1)
		return MOD_CONT;

	const char *newav[4];
	newav[0] = source;
	newav[1] = av[0];
	do_umode(source, ac, newav);
	return MOD_CONT;
}

int anope_event_kill(const char *source, int ac, const char **av)
{
	if (ac != 2)
		return MOD_CONT;

	m_kill(av[0], av[1]);
	return MOD_CONT;
}

int anope_event_kick(const char *source, int ac, const char **av)
{
	if (ac != 3)
		return MOD_CONT;
	do_kick(source, ac, av);
	return MOD_CONT;
}


int anope_event_join(const char *source, int ac, const char **av)
{
	if (ac != 1)
		return MOD_CONT;
	do_join(source, ac, av);
	return MOD_CONT;
}

int anope_event_motd(const char *source, int ac, const char **av)
{
	if (!source) {
		return MOD_CONT;
	}

	m_motd(source);
	return MOD_CONT;
}

int anope_event_setname(const char *source, int ac, const char **av)
{
	User *u;

	if (ac != 1)
		return MOD_CONT;

	u = finduser(source);
	if (!u) {
		if (debug) {
			alog("debug: SETNAME for nonexistent user %s", source);
		}
		return MOD_CONT;
	}

	u->SetRealname(av[0]);
	return MOD_CONT;
}

int anope_event_chgname(const char *source, int ac, const char **av)
{
	User *u;

	if (ac != 2)
		return MOD_CONT;

	u = finduser(av[0]);
	if (!u) {
		if (debug) {
			alog("debug: CHGNAME for nonexistent user %s", av[0]);
		}
		return MOD_CONT;
	}

	u->SetRealname(av[1]);
	return MOD_CONT;
}

int anope_event_setident(const char *source, int ac, const char **av)
{
	User *u;

	if (ac != 1)
		return MOD_CONT;

	u = finduser(source);
	if (!u) {
		if (debug) {
			alog("debug: SETIDENT for nonexistent user %s", source);
		}
		return MOD_CONT;
	}

	u->SetIdent(av[0]);
	return MOD_CONT;
}
int anope_event_chgident(const char *source, int ac, const char **av)
{
	User *u;

	if (ac != 2)
		return MOD_CONT;

	u = finduser(av[0]);
	if (!u) {
		if (debug) {
			alog("debug: CHGIDENT for nonexistent user %s", av[0]);
		}
		return MOD_CONT;
	}

	u->SetIdent(av[1]);
	return MOD_CONT;
}

int anope_event_sethost(const char *source, int ac, const char **av)
{
	User *u;

	if (ac != 1)
		return MOD_CONT;

	u = finduser(source);
	if (!u) {
		if (debug) {
			alog("debug: SETHOST for nonexistent user %s", source);
		}
		return MOD_CONT;
	}

	/* If a user has a custom host and a server splits and reconnects
	 * Unreal does not send the users cloaked host to Anope.. so we do not know it.
	 * However, they will be +t if this is the case, so we will set their vhost
	 * to the sethost value (which really is their vhost) and clear the chost.
	 * The chost will be request later (if needed) - Adam
	 */
	if (u->HasMode(UMODE_VHOST))
	{
		u->SetDisplayedHost(av[0]);
		u->chost.clear();
	}
	/* If the new host doesn't match the real host or ip.. set it
	 * else we will not set a cloaked host, and request it later if needed
	 */
	else if ((u->host && strcmp(av[0], u->host)) || (u->hostip && strcmp(av[0], u->hostip)))
	{
		u->SetCloakedHost(av[0]);
	}

	return MOD_CONT;
}

/*
** NICK - new
**	  source  = NULL
**	parv[0] = nickname
**	  parv[1] = hopcount
**	  parv[2] = timestamp
**	  parv[3] = username
**	  parv[4] = hostname
**	  parv[5] = servername
**  if NICK version 1:
**	  parv[6] = servicestamp
**	parv[7] = info
**  if NICK version 2:
**	parv[6] = servicestamp
**	  parv[7] = umodes
**	parv[8] = virthost, * if none
**	parv[9] = info
**  if NICKIP:
**	  parv[9] = ip
**	  parv[10] = info
**
** NICK - change
**	  source  = oldnick
**	parv[0] = new nickname
**	  parv[1] = hopcount
*/
/*
 do_nick(const char *source, char *nick, char *username, char *host,
			  char *server, char *realname, time_t ts,
			  uint32 ip, char *vhost, char *uid)
*/
int anope_event_nick(const char *source, int ac, const char **av)
{
	User *user;

	if (ac != 2) {
		if (ac == 7) {
			/*
			   <codemastr> that was a bug that is now fixed in 3.2.1
			   <codemastr> in  some instances it would use the non-nickv2 format
			   <codemastr> it's sent when a nick collision occurs
			   - so we have to leave it around for now -TSL
			 */
			do_nick(source, av[0], av[3], av[4], av[5], av[6],
					strtoul(av[2], NULL, 10), 0, "*", NULL);

		} else if (ac == 11) {
			user = do_nick(source, av[0], av[3], av[4], av[5], av[10],
						   strtoul(av[2], NULL, 10), ntohl(decode_ip(av[9])), av[8], NULL);
			if (user)
			{
				/* Check to see if the user should be identified because their
				 * services id matches the one in their nickcore
				 */
				user->CheckAuthenticationToken(av[6]);

				UserSetInternalModes(user, 1, &av[7]);
			}

		} else {
			/* NON NICKIP */
			user = do_nick(source, av[0], av[3], av[4], av[5], av[9],
						   strtoul(av[2], NULL, 10), 0, av[8],
						   NULL);
			if (user)
			{
				/* Check to see if the user should be identified because their
				 * services id matches the one in their nickcore
				 */
				user->CheckAuthenticationToken(av[6]);

				UserSetInternalModes(user, 1, &av[7]);
			}
		}
	} else {
		do_nick(source, av[0], NULL, NULL, NULL, NULL,
				strtoul(av[1], NULL, 10), 0, NULL, NULL);
	}
	return MOD_CONT;
}


int anope_event_chghost(const char *source, int ac, const char **av)
{
	User *u;

	if (ac != 2)
		return MOD_CONT;

	u = finduser(av[0]);
	if (!u) {
		if (debug) {
			alog("debug: CHGHOST for nonexistent user %s", av[0]);
		}
		return MOD_CONT;
	}

	u->SetDisplayedHost(av[1]);
	return MOD_CONT;
}

/* EVENT: SERVER */
int anope_event_server(const char *source, int ac, const char **av)
{
	char *desc;
	char *vl;
	char *upnumeric;

	if (!stricmp(av[1], "1")) {
		uplink = sstrdup(av[0]);
		vl = myStrGetToken(av[2], ' ', 0);
		upnumeric = myStrGetToken(vl, '-', 2);
		desc = myStrGetTokenRemainder(av[2], ' ', 1);
		do_server(source, av[0], av[1], desc, upnumeric);
		delete [] vl;
		delete [] desc;
		delete [] upnumeric;
	} else {
		do_server(source, av[0], av[1], av[2], NULL);
	}

	return MOD_CONT;
}

int anope_event_privmsg(const char *source, int ac, const char **av)
{
	if (ac != 2)
		return MOD_CONT;
	m_privmsg(source, av[0], av[1]);
	return MOD_CONT;
}

int anope_event_part(const char *source, int ac, const char **av)
{
	if (ac < 1 || ac > 2)
		return MOD_CONT;
	do_part(source, ac, av);
	return MOD_CONT;
}

int anope_event_whois(const char *source, int ac, const char **av)
{
	if (source && ac >= 1) {
		m_whois(source, av[0]);
	}
	return MOD_CONT;
}


int anope_event_error(const char *source, int ac, const char **av)
{
	if (av[0]) {
		if (debug) {
			alog("debug: %s", av[0]);
		}
	if(strstr(av[0],"No matching link configuration")!=0) {
		alog("Error: Your IRCD's link block may not setup correctly, please check unrealircd.conf");
	}
	}
	return MOD_CONT;
}

int anope_event_sdesc(const char *source, int ac, const char **av)
{
	Server *s;
	s = findserver(servlist, source);

	if (s) {
		s->desc = const_cast<char *>(av[0]); // XXX Unsafe cast -- CyberBotX
	}

	return MOD_CONT;
}

int anope_event_userhost(const char *source, int ac, const char **av)
{
	/** Hack to get around Unreal:
	 * This is the USERHOST reply, we only send a request if we do not know the users cloaked host
	 * (they got introducted using a vhost) - Adam
	 */
	if (ac < 2 || !av[1] || !*av[1])
		return MOD_CONT;

	std::string reply = av[1];
	std::string user = std::string(reply.begin(), std::find(reply.begin(), reply.end(), '='));
	if (user[user.length() - 1] == '*')
		user.erase(user.length() - 1);
	std::string host = std::string(std::find(reply.begin(), reply.end(), '@'), reply.end());
	host.erase(host.begin());

	User *u = finduser(user);
	if (u)
	{
		u->SetCloakedHost(host);
	}

	return MOD_CONT;
}

int anope_event_sjoin(const char *source, int ac, const char **av)
{
	do_sjoin(source, ac, av);
	return MOD_CONT;
}

void moduleAddIRCDMsgs() {
	Message *m;

	m = createMessage("436",	   anope_event_436); addCoreMessage(IRCD,m);
	m = createMessage("AWAY",	  anope_event_away); addCoreMessage(IRCD,m);
	m = createMessage("6",		anope_event_away); addCoreMessage(IRCD,m);
	m = createMessage("JOIN",	  anope_event_join); addCoreMessage(IRCD,m);
	m = createMessage("C",		anope_event_join); addCoreMessage(IRCD,m);
	m = createMessage("KICK",	  anope_event_kick); addCoreMessage(IRCD,m);
	m = createMessage("H",		anope_event_kick); addCoreMessage(IRCD,m);
	m = createMessage("KILL",	  anope_event_kill); addCoreMessage(IRCD,m);
	m = createMessage(".",		anope_event_kill); addCoreMessage(IRCD,m);
	m = createMessage("MODE",	  anope_event_mode); addCoreMessage(IRCD,m);
	m = createMessage("G",		anope_event_gmode); addCoreMessage(IRCD,m);
	m = createMessage("MOTD",	  anope_event_motd); addCoreMessage(IRCD,m);
	m = createMessage("F",		anope_event_motd); addCoreMessage(IRCD,m);
	m = createMessage("NICK",	  anope_event_nick); addCoreMessage(IRCD,m);
	m = createMessage("&",		anope_event_nick); addCoreMessage(IRCD,m);
	m = createMessage("PART",	  anope_event_part); addCoreMessage(IRCD,m);
	m = createMessage("D",		anope_event_part); addCoreMessage(IRCD,m);
	m = createMessage("PING",	  anope_event_ping); addCoreMessage(IRCD,m);
	m = createMessage("8",		anope_event_ping); addCoreMessage(IRCD,m);
	m = createMessage("PRIVMSG",   anope_event_privmsg); addCoreMessage(IRCD,m);
	m = createMessage("!",		anope_event_privmsg); addCoreMessage(IRCD,m);
	m = createMessage("QUIT",	  anope_event_quit); addCoreMessage(IRCD,m);
	m = createMessage(",",		anope_event_quit); addCoreMessage(IRCD,m);
	m = createMessage("SERVER",	anope_event_server); addCoreMessage(IRCD,m);
	m = createMessage("'",		anope_event_server); addCoreMessage(IRCD,m);
	m = createMessage("SQUIT",	 anope_event_squit); addCoreMessage(IRCD,m);
	m = createMessage("-",		anope_event_squit); addCoreMessage(IRCD,m);
	m = createMessage("TOPIC",	 anope_event_topic); addCoreMessage(IRCD,m);
	m = createMessage(")",		anope_event_topic); addCoreMessage(IRCD,m);
	m = createMessage("SVSMODE",   anope_event_mode); addCoreMessage(IRCD,m);
	m = createMessage("n",		anope_event_mode); addCoreMessage(IRCD,m);
	m = createMessage("SVS2MODE",   anope_event_mode); addCoreMessage(IRCD,m);
	m = createMessage("v",	   anope_event_mode); addCoreMessage(IRCD,m);
	m = createMessage("WHOIS",	 anope_event_whois); addCoreMessage(IRCD,m);
	m = createMessage("#",		anope_event_whois); addCoreMessage(IRCD,m);
	m = createMessage("PROTOCTL",  anope_event_capab); addCoreMessage(IRCD,m);
	m = createMessage("_",		anope_event_capab); addCoreMessage(IRCD,m);
	m = createMessage("CHGHOST",   anope_event_chghost); addCoreMessage(IRCD,m);
	m = createMessage("AL",		anope_event_chghost); addCoreMessage(IRCD,m);
	m = createMessage("CHGIDENT",  anope_event_chgident); addCoreMessage(IRCD,m);
	m = createMessage("AZ",		anope_event_chgident); addCoreMessage(IRCD,m);
	m = createMessage("CHGNAME",   anope_event_chgname); addCoreMessage(IRCD,m);
	m = createMessage("BK",		anope_event_chgname); addCoreMessage(IRCD,m);
	m = createMessage("NETINFO",   anope_event_netinfo); addCoreMessage(IRCD,m);
	m = createMessage("AO",	   anope_event_netinfo); addCoreMessage(IRCD,m);
	m = createMessage("SETHOST",   anope_event_sethost); addCoreMessage(IRCD,m);
	m = createMessage("AA",		anope_event_sethost); addCoreMessage(IRCD,m);
	m = createMessage("SETIDENT",  anope_event_setident); addCoreMessage(IRCD,m);
	m = createMessage("AD",		anope_event_setident); addCoreMessage(IRCD,m);
	m = createMessage("SETNAME",   anope_event_setname); addCoreMessage(IRCD,m);
	m = createMessage("AE",		anope_event_setname); addCoreMessage(IRCD,m);
	m = createMessage("EOS", 	   anope_event_eos); addCoreMessage(IRCD,m);
	m = createMessage("ES",	   anope_event_eos); addCoreMessage(IRCD,m);
	m = createMessage("ERROR", 	   anope_event_error); addCoreMessage(IRCD,m);
	m = createMessage("5",		anope_event_error); addCoreMessage(IRCD,m);
	m = createMessage("UMODE2",	anope_event_umode2); addCoreMessage(IRCD,m);
	m = createMessage("|",		anope_event_umode2); addCoreMessage(IRCD,m);
	m = createMessage("SJOIN",	  anope_event_sjoin); addCoreMessage(IRCD,m);
	m = createMessage("~",		anope_event_sjoin); addCoreMessage(IRCD,m);
	m = createMessage("SDESC",	  anope_event_sdesc); addCoreMessage(IRCD,m);
	m = createMessage("AG",	   anope_event_sdesc); addCoreMessage(IRCD,m);
	m = createMessage("302",	anope_event_userhost); addCoreMessage(IRCD,m);

	/* The non token version of these is in messages.c */
	m = createMessage("2",		 m_stats); addCoreMessage(IRCD,m);
	m = createMessage(">",		 m_time); addCoreMessage(IRCD,m);
	m = createMessage("+",		 m_version); addCoreMessage(IRCD,m);
}

/* Borrowed part of this check from UnrealIRCd */
bool ChannelModeFlood::IsValid(const char *value)
{
	char *dp, *end;
	/* NEW +F */
	char xbuf[256], *p, *p2, *x = xbuf + 1;
	int v;
	if (!value) return 0;
	if (*value != ':' && strtoul((*value == '*' ? value + 1 : value), &dp, 10) > 0 && *dp == ':' && *(++dp) && strtoul(dp, &end, 10) > 0 && !*end) return 1;
	else {
		/* '['<number><1 letter>[optional: '#'+1 letter],[next..]']'':'<number> */
		strlcpy(xbuf, value, sizeof(xbuf));
		p2 = strchr(xbuf + 1, ']');
		if (!p2) return 0;
		*p2 = '\0';
		if (*(p2 + 1) != ':') return 0;
		for (x = strtok(xbuf + 1, ","); x; x = strtok(NULL, ",")) {
			/* <number><1 letter>[optional: '#'+1 letter] */
			p = x;
			while (isdigit(*p)) ++p;
			if (!*p || !(*p == 'c' || *p == 'j' || *p == 'k' || *p == 'm' || *p == 'n' || *p == 't')) continue; /* continue instead of break for forward compatability. */
			*p = '\0';
			v = atoi(x);
			if (v < 1 || v > 999) return 0;
			++p;
		}
		return 1;
	}
}

void moduleAddModes()
{
	/* Add user modes */
	ModeManager::AddUserMode('A', new UserMode(UMODE_SERV_ADMIN));
	ModeManager::AddUserMode('B', new UserMode(UMODE_BOT));
	ModeManager::AddUserMode('C', new UserMode(UMODE_CO_ADMIN));
	ModeManager::AddUserMode('G', new UserMode(UMODE_FILTER));
	ModeManager::AddUserMode('H', new UserMode(UMODE_HIDEOPER));
	ModeManager::AddUserMode('N', new UserMode(UMODE_NETADMIN));
	ModeManager::AddUserMode('R', new UserMode(UMODE_REGPRIV));
	ModeManager::AddUserMode('S', new UserMode(UMODE_PROTECTED));
	ModeManager::AddUserMode('T', new UserMode(UMODE_NO_CTCP));
	ModeManager::AddUserMode('V', new UserMode(UMODE_WEBTV));
	ModeManager::AddUserMode('W', new UserMode(UMODE_WHOIS));
	ModeManager::AddUserMode('a', new UserMode(UMODE_ADMIN));
	ModeManager::AddUserMode('d', new UserMode(UMODE_DEAF));
	ModeManager::AddUserMode('g', new UserMode(UMODE_GLOBOPS));
	ModeManager::AddUserMode('h', new UserMode(UMODE_HELPOP));
	ModeManager::AddUserMode('i', new UserMode(UMODE_INVIS));
	ModeManager::AddUserMode('o', new UserMode(UMODE_OPER));
	ModeManager::AddUserMode('p', new UserMode(UMODE_PRIV));
	ModeManager::AddUserMode('q', new UserMode(UMODE_GOD));
	ModeManager::AddUserMode('r', new UserMode(UMODE_REGISTERED));
	ModeManager::AddUserMode('s', new UserMode(UMODE_SNOMASK));
	ModeManager::AddUserMode('t', new UserMode(UMODE_VHOST));
	ModeManager::AddUserMode('w', new UserMode(UMODE_WALLOPS));
	ModeManager::AddUserMode('x', new UserMode(UMODE_CLOAK));
	ModeManager::AddUserMode('z', new UserMode(UMODE_SSL));

	/* b/e/I */
	ModeManager::AddChannelMode('b', new ChannelModeBan());
	ModeManager::AddChannelMode('e', new ChannelModeExcept());
	ModeManager::AddChannelMode('I', new ChannelModeInvite());

	/* v/h/o/a/q */
	ModeManager::AddChannelMode('v', new ChannelModeStatus(CMODE_VOICE, CUS_VOICE, '+'));
	ModeManager::AddChannelMode('h', new ChannelModeStatus(CMODE_HALFOP, CUS_HALFOP, '%'));
	ModeManager::AddChannelMode('o', new ChannelModeStatus(CMODE_OP, CUS_OP, '@', true));
	ModeManager::AddChannelMode('a', new ChannelModeStatus(CMODE_PROTECT, CUS_PROTECT, '&', true));
	/* Unreal sends +q as * */
	ModeManager::AddChannelMode('q', new ChannelModeStatus(CMODE_OWNER, CUS_OWNER, '*'));

	/* Add channel modes */
	ModeManager::AddChannelMode('c', new ChannelMode(CMODE_BLOCKCOLOR));
	ModeManager::AddChannelMode('f', new ChannelModeFlood());
	ModeManager::AddChannelMode('i', new ChannelMode(CMODE_INVITE));
	ModeManager::AddChannelMode('j', new ChannelModeParam(CMODE_JOINFLOOD, true));
	ModeManager::AddChannelMode('k', new ChannelModeKey());
	ModeManager::AddChannelMode('l', new ChannelModeParam(CMODE_LIMIT, true));
	ModeManager::AddChannelMode('m', new ChannelMode(CMODE_MODERATED));
	ModeManager::AddChannelMode('n', new ChannelMode(CMODE_NOEXTERNAL));
	ModeManager::AddChannelMode('p', new ChannelMode(CMODE_PRIVATE));
	ModeManager::AddChannelMode('r', new ChannelModeRegistered());
	ModeManager::AddChannelMode('s', new ChannelMode(CMODE_SECRET));
	ModeManager::AddChannelMode('t', new ChannelMode(CMODE_TOPIC));
	ModeManager::AddChannelMode('u', new ChannelMode(CMODE_AUDITORIUM));
	ModeManager::AddChannelMode('z', new ChannelMode(CMODE_SSL));
	ModeManager::AddChannelMode('A', new ChannelModeAdmin());
	ModeManager::AddChannelMode('C', new ChannelMode(CMODE_NOCTCP));
	ModeManager::AddChannelMode('G', new ChannelMode(CMODE_FILTER));
	ModeManager::AddChannelMode('K', new ChannelMode(CMODE_NOKNOCK));
	ModeManager::AddChannelMode('L', new ChannelModeParam(CMODE_REDIRECT));
	ModeManager::AddChannelMode('M', new ChannelMode(CMODE_REGMODERATED));
	ModeManager::AddChannelMode('N', new ChannelMode(CMODE_NONICK));
	ModeManager::AddChannelMode('O', new ChannelModeOper());
	ModeManager::AddChannelMode('Q', new ChannelMode(CMODE_NOKICK));
	ModeManager::AddChannelMode('R', new ChannelMode(CMODE_REGISTEREDONLY));
	ModeManager::AddChannelMode('S', new ChannelMode(CMODE_STRIPCOLOR));
	ModeManager::AddChannelMode('T', new ChannelMode(CMODE_NONOTICE));
	ModeManager::AddChannelMode('V', new ChannelMode(CMODE_NOINVITE));
}

class ProtoUnreal : public Module
{
 public:
	ProtoUnreal(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(PROTOCOL);

		pmodule_ircd_version("UnrealIRCd 3.2+");
		pmodule_ircd_cap(myIrcdcap);
		pmodule_ircd_var(myIrcd);
		pmodule_ircd_useTSMode(0);

		moduleAddModes();

		pmodule_ircd_proto(&ircd_proto);
		moduleAddIRCDMsgs();
	}

};

MODULE_INIT(ProtoUnreal)

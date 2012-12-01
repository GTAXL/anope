/*
 * ngIRCd Protocol module for Anope IRC Services
 *
 * (C) 2012 Anope Team <team@anope.org>
 * (C) 2011-2012 Alexander Barton <alex@barton.de>
 * (C) 2011 Anope Team <team@anope.org>
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

#include "module.h"

class ngIRCdProto : public IRCDProto
{
 public:
	ngIRCdProto(Module *creator) : IRCDProto(creator, "ngIRCd")
	{
		DefaultPseudoclientModes = "+oi";
		CanSVSNick = true;
		CanSetVHost = true;
		CanSetVIdent = true;
		MaxModes = 5;
	}

	void SendAkill(User *u, XLine *x) anope_override
	{
		// Calculate the time left before this would expire, capping it at 2 days
		time_t timeleft = x->expires - Anope::CurTime;
		if (timeleft > 172800 || !x->expires)
			timeleft = 172800;
		UplinkSocket::Message(Me) << "GLINE " << x->mask << " " << timeleft << " :" << x->GetReason() << " (" << x->by << ")";
	}

	void SendAkillDel(const XLine *x) anope_override
	{
		UplinkSocket::Message(Me) << "GLINE " << x->mask;
	}

	void SendChannel(Channel *c) anope_override
	{
		UplinkSocket::Message(Me) << "CHANINFO " << c->name << " +" << c->GetModes(true, true);
	}

	// Received: :dev.anope.de NICK DukeP 1 ~DukePyro p57ABF9C9.dip.t-dialin.net 1 +i :DukePyrolator
	void SendClientIntroduction(const User *u) anope_override
	{
		Anope::string modes = "+" + u->GetModes();
		UplinkSocket::Message(Me) << "NICK " << u->nick << " 1 " << u->GetIdent() << " " << u->host << " 1 " << modes << " :" << u->realname;
	}

	void SendConnect() anope_override
	{
		UplinkSocket::Message() << "PASS " << Config->Uplinks[Anope::CurrentUplink]->password << " 0210-IRC+ Anope|" << Anope::VersionShort() << ":CLHMSo P";
		/* Make myself known to myself in the serverlist */
		SendServer(Me);
		/* finish the enhanced server handshake and register the connection */
		this->SendNumeric(376, "*", ":End of MOTD command");
	}

	void SendForceNickChange(const User *u, const Anope::string &newnick, time_t when) anope_override
	{
		UplinkSocket::Message(Me) << "SVSNICK " << u->nick << " " << newnick;
	}

	void SendGlobalNotice(const BotInfo *bi, const Server *dest, const Anope::string &msg) anope_override
	{
		UplinkSocket::Message(bi) << "NOTICE $" << dest->GetName() << " :" << msg;
	}

	void SendGlobalPrivmsg(const BotInfo *bi, const Server *dest, const Anope::string &msg) anope_override
	{
		UplinkSocket::Message(bi) << "PRIVMSG $" << dest->GetName() << " :" << msg;
	}

	void SendGlobopsInternal(const BotInfo *source, const Anope::string &buf) anope_override
	{
		if (source)
			UplinkSocket::Message(source) << "WALLOPS :" << buf;
		else
			UplinkSocket::Message(Me) << "WALLOPS :" << buf;
	}

	void SendJoin(const User *user, Channel *c, const ChannelStatus *status) anope_override
	{
		UplinkSocket::Message(user) << "JOIN " << c->name;
		if (status)
		{
			/* First save the channel status incase uc->Status == status */
			ChannelStatus cs = *status;
			/* If the user is internally on the channel with flags, kill them so that
			 * the stacker will allow this.
			 */
			UserContainer *uc = c->FindUser(user);
			if (uc != NULL)
				uc->status->ClearFlags();

			BotInfo *setter = BotInfo::Find(user->nick);
			for (unsigned i = 0; i < ModeManager::ChannelModes.size(); ++i)
				if (cs.HasFlag(ModeManager::ChannelModes[i]->name))
					c->SetMode(setter, ModeManager::ChannelModes[i], user->GetUID(), false);
		}
	}

	void SendKickInternal(const BotInfo *bi, const Channel *chan, const User *user, const Anope::string &buf) anope_override
	{
		if (!buf.empty())
			UplinkSocket::Message(bi) << "KICK " << chan->name << " " << user->nick << " :" << buf;
		else
			UplinkSocket::Message(bi) << "KICK " << chan->name << " " << user->nick;
	}

	void SendLogin(User *u) anope_override { }

	void SendLogout(User *u) anope_override { } 

	void SendModeInternal(const BotInfo *bi, const Channel *dest, const Anope::string &buf) anope_override
	{
		if (bi)
			UplinkSocket::Message(bi) << "MODE " << dest->name << " " << buf;
		else
			UplinkSocket::Message(Me) << "MODE " << dest->name << " " << buf;
	}

	void SendModeInternal(const BotInfo *bi, const User *u, const Anope::string &buf) anope_override
	{
		if (bi)
			UplinkSocket::Message(bi) << "MODE " << u->nick << " " << buf;
		else
			UplinkSocket::Message(Me) << "MODE " << u->nick << " " << buf;
	}

	void SendPartInternal(const BotInfo *bi, const Channel *chan, const Anope::string &buf) anope_override
	{
		if (!buf.empty())
			UplinkSocket::Message(bi) << "PART " << chan->name << " :" << buf;
		else
			UplinkSocket::Message(bi) << "PART " << chan->name;
	}

	/* SERVER name hop descript */
	void SendServer(const Server *server) anope_override
	{
		UplinkSocket::Message() << "SERVER " << server->GetName() << " " << server->GetHops() << " :" << server->GetDescription();
	}

	void SendTopic(BotInfo *bi, Channel *c) anope_override
	{
		UplinkSocket::Message(bi) << "TOPIC " << c->name << " :" << c->topic;
	}

	void SendVhost(User *u, const Anope::string &vIdent, const Anope::string &vhost) anope_override
	{
		if (!vIdent.empty())
			UplinkSocket::Message(Me) << "METADATA " << u->nick << " user :" << vIdent;
		if (!vhost.empty())
		{
			if (!u->HasMode(UMODE_CLOAK))
			{
				u->SetMode(HostServ, UMODE_CLOAK);
				// send the modechange before we send the vhost
				ModeManager::ProcessModes();
			}
			UplinkSocket::Message(Me) << "METADATA " << u->nick << " host :" << vhost;
		}
	}

	void SendVhostDel(User *u) anope_override
	{
		this->SendVhost(u, u->GetIdent(), u->GetCloakedHost());
	}
};

struct IRCDMessage005 : IRCDMessage
{
	IRCDMessage005(Module *creator) : IRCDMessage(creator, "005", 1) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); }

	// Please see <http://www.irc.org/tech_docs/005.html> for details.
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		size_t pos;
		Anope::string parameter, data;
		for (unsigned i = 0, end = params.size(); i < end; ++i)
		{
			pos = params[i].find('=');
			if (pos != Anope::string::npos)
			{
				parameter = params[i].substr(0, pos);
				data = params[i].substr(pos+1, params[i].length());
				if (parameter == "MODES")
				{
					unsigned maxmodes = convertTo<unsigned>(data);
					IRCD->MaxModes = maxmodes;
				}
				else if (parameter == "NICKLEN")
				{
					unsigned newlen = convertTo<unsigned>(data);
					if (Config->NickLen != newlen)
					{
						Log() << "NickLen changed from " << Config->NickLen << " to " << newlen;
						Config->NickLen = newlen;
					}
				}
			}
		}
	}
};

struct IRCDMessage376 : IRCDMessage
{
	IRCDMessage376(Module *creator) : IRCDMessage(creator, "376", 2) { }

	/*
	 * :ngircd.dev.anope.de 376 services.anope.de :End of MOTD command
	 *
	 * we do nothing here, this function exists only to
	 * avoid the "unknown message from server" message.
	 *
	 */

	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
	}
};

struct IRCDMessageChaninfo : IRCDMessage
{
	IRCDMessageChaninfo(Module *creator) : IRCDMessage(creator, "CHANINFO", 2) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); SetFlag(IRCDMESSAGE_REQUIRE_SERVER); }

	/*
	 * CHANINFO is used by servers to inform each other about a channel: its
	 * modes, channel key, user limits and its topic. The parameter combination
	 * <key> and <limit> is optional, as well as the <topic> parameter, so that
	 * there are three possible forms of this command:
	 *
	 * CHANINFO <chan> +<modes>
	 * CHANINFO <chan> +<modes> :<topic>
	 * CHANINFO <chan> +<modes> <key> <limit> :<topic>
	 *
	 * The parameter <key> must be ignored if a channel has no key (the parameter
	 * <modes> doesn't list the "k" channel mode). In this case <key> should
	 * contain "*" because the parameter <key> is required by the CHANINFO syntax
	 * and therefore can't be omitted. The parameter <limit> must be ignored when
	 * a channel has no user limit (the parameter <modes> doesn't list the "l"
	 * channel mode). In this case <limit> should be "0".
	 */
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{

		Channel *c = Channel::Find(params[0]);
		if (!c)
			c = new Channel(params[0]);

		Anope::string modes = params[1];

		if (params.size() == 3)
		{
			c->ChangeTopicInternal(source.GetName(), params[2], Anope::CurTime);
		}
		else if (params.size() == 5)
		{
			for (size_t i = 0, end = params[1].length(); i < end; ++i)
			{
				switch(params[1][i])
				{
					case 'k':
						modes += " " + params[2];
						continue;
					case 'l':
						modes += " " + params[3];
						continue;
			}
		}
			c->ChangeTopicInternal(source.GetName(), params[4], Anope::CurTime);
		}

		c->SetModesInternal(source, modes);
	}
};

struct IRCDMessageJoin : Message::Join
{
	IRCDMessageJoin(Module *creator) : Message::Join(creator, "JOIN") { }

	/*
	 * <@po||ux> DukeP: RFC 2813, 4.2.1: the JOIN command on server-server links
	 * separates the modes ("o") with ASCII 7, not space. And you can't see ASCII 7.
	 *
	 * if a user joins a new channel, the ircd sends <channelname>\7<umode>
	 */
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		User *user = source.GetUser();
		size_t pos = params[0].find('\7');
		Anope::string channel, modes;

		if (pos != Anope::string::npos)
		{
			channel = params[0].substr(0, pos);
			modes = '+' + params[0].substr(pos+1, params[0].length()) + " " + user->nick;
		}
		else
		{
			channel = params[0];
		}

		std::vector<Anope::string> new_params;
		new_params.push_back(channel);

		Message::Join::Run(source, new_params);

		if (!modes.empty())
		{
			Channel *c = Channel::Find(channel);
			if (c)
				c->SetModesInternal(source, modes);
		}
	}
};

struct IRCDMessageMetadata : IRCDMessage
{
	IRCDMessageMetadata(Module *creator) : IRCDMessage(creator, "METADATA", 3) { SetFlag(IRCDMESSAGE_REQUIRE_SERVER); }

	/*
	 * Received: :ngircd.dev.anope.de METADATA DukePyrolator host :anope-e2ee5c7d
	 *
	 * params[0] = nick of the user
	 * params[1] = command
	 * params[3] = data
	 *
	 * following commands are supported:
	 *  - "host": the hostname of a client (can't be empty)
	 *  - "info": info text ("real name") of a client
	 *  - "user": the user name (ident) of a client (can't be empty)
	 */

	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		User *u = User::Find(params[0]);
		if (!u)
		{
			Log() << "received METADATA for non-existent user " << params[0];
			return;
		}
		if (params[1].equals_cs("host"))
		{
			u->SetCloakedHost(params[2]);
		}
		else if (params[1].equals_cs("info"))
		{
			u->SetRealname(params[2]);
		}
		else if (params[1].equals_cs("user"))
		{
			u->SetVIdent(params[2]);
		}
	}
};

struct IRCDMessageMode : IRCDMessage
{
	IRCDMessageMode(Module *creator) : IRCDMessage(creator, "MODE", 2) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); }

	/*
	 * Received: :DukeP MODE #anope +b *!*@*.aol.com
	 * Received: :DukeP MODE #anope +h DukeP
	 * params[0] = channel or nick
	 * params[1] = modes
	 * params[n] = parameters
	 */

	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		Anope::string modes = params[1];

		for (size_t i = 2; i < params.size(); ++i)
			modes += " " + params[i];

		if (IRCD->IsChannelValid(params[0]))
		{
			Channel *c = Channel::Find(params[0]);

			if (c)
				c->SetModesInternal(source, modes);
		}
		else
		{
			User *u = User::Find(params[0]);

			if (u)
				u->SetModesInternal("%s", params[1].c_str());
		}
	}
};

struct IRCDMessageNick : IRCDMessage
{
	IRCDMessageNick(Module *creator) : IRCDMessage(creator, "NICK", 1) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); }

	/*
	 * NICK - NEW
	 * Received: :dev.anope.de NICK DukeP_ 1 ~DukePyro ip-2-201-236-154.web.vodafone.de 1 + :DukePyrolator
	 * Parameters: <nickname> <hopcount> <username> <host> <servertoken> <umode> :<realname>
	 * source = server
	 * params[0] = nick
	 * params[1] = hopcount
	 * params[2] = username/ident
	 * params[3] = host
	 * params[4] = servertoken
	 * params[5] = modes
	 * params[6] = info
	 *
	 * NICK - change
	 * Received: :DukeP_ NICK :test2
	 * source    = oldnick
	 * params[0] = newnick
	 *
	 */
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		if (params.size() == 1)
		{
			// we have a nickchange
			source.GetUser()->ChangeNick(params[0]);
		}
		else if (params.size() == 7)
		{
			// a new user is connecting to the network
			User *user = new User(params[0], params[2], params[3], "", "", source.GetServer(), params[6], Anope::CurTime, params[5], "");
			if (user && NickServService)
				NickServService->Validate(user);
		}
		else
		{
			Log(LOG_DEBUG) << "Received NICK with invalid number of parameters. source = " << source.GetName() << "params[0] = " << params[0] << "params.size() = " << params.size();
		}
	}
};

struct IRCDMessageNJoin : IRCDMessage
{
	IRCDMessageNJoin(Module *creator) : IRCDMessage(creator, "NJOIN",2) { SetFlag(IRCDMESSAGE_REQUIRE_SERVER); };

	/*
	 * RFC 2813, 4.2.2: Njoin Message:
	 * The NJOIN message is used between servers only.
	 * It is used when two servers connect to each other to exchange
	 * the list of channel members for each channel.
	 *
	 * Even though the same function can be performed by using a succession
	 * of JOIN, this message SHOULD be used instead as it is more efficient.
	 *
	 * Received: :dev.anope.de NJOIN #test :DukeP2,@DukeP,%test,+test2
	 */
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		std::list<Message::Join::SJoinUser> users;

		commasepstream sep(params[1]);
		Anope::string buf;
		while (sep.GetToken(buf))
		{

			Message::Join::SJoinUser sju;

			/* Get prefixes from the nick */
			for (char ch; (ch = ModeManager::GetStatusChar(buf[0]));)
			{
				ChannelMode *cm = ModeManager::FindChannelModeByChar(ch);
				buf.erase(buf.begin());
				if (!cm)
				{
					Log(LOG_DEBUG) << "Received unknown mode prefix " << ch << " in NJOIN string.";
					continue;
				}

				sju.first.SetFlag(cm->name);
			}

			sju.second = User::Find(buf);
			if (!sju.second)
			{
				Log(LOG_DEBUG) << "NJOIN for nonexistant user " << buf << " on " << params[0];
				continue;
			}
		} 

		Message::Join::SJoin(source, params[0], 0, "", users);
	}
};

struct IRCDMessagePong : IRCDMessage
{
	IRCDMessagePong(Module *creator) : IRCDMessage(creator, "PONG", 0) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); SetFlag(IRCDMESSAGE_REQUIRE_SERVER); }

	/*
	 * ngIRCd does not send an EOB, so we send a PING immediately
	 * when receiving a new server and then finish sync once we
	 * get a pong back from that server.
	 */
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		if (!source.GetServer()->IsSynced())
			source.GetServer()->Sync(false);
	}
};

struct IRCDMessageServer : IRCDMessage
{
	IRCDMessageServer(Module *creator) : IRCDMessage(creator, "SERVER", 3) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); }

	/*
	 * SERVER tolsun.oulu.fi 1 :Experimental server
	 * 	New server tolsun.oulu.fi introducing itself
	 * 	and attempting to register.
	 *
	 * RFC 2813 says the server has to send a hopcount
	 * AND a servertoken. Not quite sure what ngIRCd is
	 * sending here.
	 *
	 * params[0] = servername
	 * params[1] = hop count (or servertoken?)
	 * params[2] = server description
	 *
	 *
	 * :tolsun.oulu.fi SERVER csd.bu.edu 5 34 :BU Central Server
	 *	Server tolsun.oulu.fi is our uplink for csd.bu.edu
	 *	which is 5 hops away. The token "34" will be used
	 *	by tolsun.oulu.fi when introducing new users or
	 *	services connected to csd.bu.edu.
	 *
	 * params[0] = servername
	 * params[1] = hop count
	 * params[2] = server numeric
	 * params[3] = server description
	 */

	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		if (params.size() == 3)
		{
			// our uplink is introducing itself
			new Server(Me, params[0], 1, params[2], "");
		}
		else
		{
			// our uplink is introducing a new server
			unsigned int hops = params[1].is_pos_number_only() ? convertTo<unsigned>(params[1]) : 0;
			new Server(source.GetServer(), params[0], hops, params[2], params[3]);
		}
		/*
		 * ngIRCd does not send an EOB, so we send a PING immediately
		 * when receiving a new server and then finish sync once we
		 * get a pong back from that server.
		 */
		IRCD->SendPing(Config->ServerName, params[0]);
	}
};

struct IRCDMessageTopic : IRCDMessage
{
	IRCDMessageTopic(Module *creator) : IRCDMessage(creator, "TOPIC", 2) { SetFlag(IRCDMESSAGE_SOFT_LIMIT); }

	// Received: :DukeP TOPIC #anope :test
	void Run(MessageSource &source, const std::vector<Anope::string> &params) anope_override
	{
		Channel *c = Channel::Find(params[0]);
		if (!c)
		{
			Log(LOG_DEBUG) << "TOPIC for nonexistant channel " << params[0];
			return;
		}
		c->ChangeTopicInternal(source.GetName(), params[1], Anope::CurTime);
	}
};



class ProtongIRCd : public Module
{
	ngIRCdProto ircd_proto;

	/* Core message handlers */
	Message::Capab message_capab;
	Message::Error message_error;
	Message::Kick message_kick;
	Message::Kill message_kill;
	Message::MOTD message_motd;
	Message::Part message_part;
	Message::Ping message_ping;
	Message::Privmsg message_privmsg, message_squery;
	Message::Quit message_quit;
	Message::SQuit message_squit;
	Message::Stats message_stats;
	Message::Time message_time;
	Message::Version message_version;

	/* Our message handlers */
	IRCDMessage005 message_005;
	IRCDMessage376 message_376;
	IRCDMessageChaninfo message_chaninfo;
	IRCDMessageJoin message_join;
	IRCDMessageMetadata message_metadata;
	IRCDMessageMode message_mode;
	IRCDMessageNick message_nick;
	IRCDMessageNJoin message_njoin;
	IRCDMessagePong message_pong;
	IRCDMessageServer message_server;
	IRCDMessageTopic message_topic;

	void AddModes()
	{
		/* Add user modes */
		ModeManager::AddUserMode(new UserMode(UMODE_NOCTCP, 'b'));
		ModeManager::AddUserMode(new UserMode(UMODE_BOT, 'B'));
		ModeManager::AddUserMode(new UserMode(UMODE_COMMONCHANS, 'C'));
		ModeManager::AddUserMode(new UserMode(UMODE_INVIS, 'i'));
		ModeManager::AddUserMode(new UserMode(UMODE_OPER, 'o'));
		ModeManager::AddUserMode(new UserMode(UMODE_PROTECTED, 'q'));
		ModeManager::AddUserMode(new UserMode(UMODE_RESTRICTED, 'r'));
		ModeManager::AddUserMode(new UserMode(UMODE_REGISTERED, 'R'));
		ModeManager::AddUserMode(new UserMode(UMODE_SNOMASK, 's'));
		ModeManager::AddUserMode(new UserMode(UMODE_WALLOPS, 'w'));
		ModeManager::AddUserMode(new UserMode(UMODE_CLOAK, 'x'));

		/* Add modes for ban, exception, and invite lists */
		ModeManager::AddChannelMode(new ChannelModeList(CMODE_BAN, 'b'));
		ModeManager::AddChannelMode(new ChannelModeList(CMODE_EXCEPT, 'e'));
		ModeManager::AddChannelMode(new ChannelModeList(CMODE_INVITEOVERRIDE, 'I'));

		/* Add channel user modes */
		ModeManager::AddChannelMode(new ChannelModeStatus(CMODE_VOICE, 'v', '+'));
		ModeManager::AddChannelMode(new ChannelModeStatus(CMODE_HALFOP, 'h', '%'));
		ModeManager::AddChannelMode(new ChannelModeStatus(CMODE_OP, 'o', '@'));
		ModeManager::AddChannelMode(new ChannelModeStatus(CMODE_PROTECT, 'a', '&'));
		ModeManager::AddChannelMode(new ChannelModeStatus(CMODE_OWNER, 'q','~'));

		/* Add channel modes */
		ModeManager::AddChannelMode(new ChannelMode(CMODE_INVITE, 'i'));
		ModeManager::AddChannelMode(new ChannelModeKey('k'));
		ModeManager::AddChannelMode(new ChannelModeParam(CMODE_LIMIT, 'l'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_MODERATED, 'm'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_REGMODERATED, 'M'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_NOEXTERNAL, 'n'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_OPERONLY, 'O'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_PERM, 'P'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_NOKICK, 'Q'));
		ModeManager::AddChannelMode(new ChannelModeRegistered('r'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_REGISTEREDONLY, 'R'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_SECRET, 's'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_TOPIC, 't'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_NOINVITE, 'V'));
		ModeManager::AddChannelMode(new ChannelMode(CMODE_SSL, 'z'));
	}

 public:
	ProtongIRCd(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, PROTOCOL),
		ircd_proto(this),
		message_capab(this), message_error(this), message_kick(this), message_kill(this), message_motd(this),
		message_part(this), message_ping(this), message_privmsg(this), message_squery(this, "SQUERY"),
		message_quit(this), message_squit(this), message_stats(this), message_time(this), message_version(this),

		message_005(this), message_376(this), message_chaninfo(this), message_join(this), message_metadata(this),
		message_mode(this), message_nick(this), message_njoin(this), message_pong(this), message_server(this),
		message_topic(this)
	{
		this->SetAuthor("Anope");

		Servers::Capab.insert("QS");

		this->AddModes();

		Implementation i[] = { I_OnUserNickChange };
		ModuleManager::Attach(i, this, sizeof(i) / sizeof(Implementation));
	}

	void OnUserNickChange(User *u, const Anope::string &) anope_override
	{
		u->RemoveModeInternal(ModeManager::FindUserModeByName(UMODE_REGISTERED));
	}
};

MODULE_INIT(ProtongIRCd)
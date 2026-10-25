// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

/*	@(#)Copyright (C) H.Shirouzu 1996-2004   ipmsg.h	Ver2.05 */
/* ========================================================================
	Project  Name			: IP Messenger for Win32
	Module Name				: Main Header
	Create					: 1996-06-01(Sat)
	Update					: 2004-01-01(Thu)
	Copyright				: H.Shirouzu
	Reference				:
	======================================================================== */

#ifndef IPMSG_H
#define IPMSG_H

#include <time.h>

/*  IP Messenger Communication Protocol version 1.2 define  */
/*  macro  */
#define GET_MODE(command)	(command & 0x000000ffUL)
#define GET_OPT(command)	(command & 0xffffff00UL)

/*  header  */
#define IPMSG_VERSION			0x0001
#define IPMSG_DEFAULT_PORT		0x0979

/*  command  */
#define IPMSG_NOOPERATION		0x00000000UL

#define IPMSG_BR_ENTRY			0x00000001UL
#define IPMSG_BR_EXIT			0x00000002UL
#define IPMSG_ANSENTRY			0x00000003UL
#define IPMSG_BR_ABSENCE		0x00000004UL

#define IPMSG_BR_ISGETLIST		0x00000010UL
#define IPMSG_OKGETLIST			0x00000011UL
#define IPMSG_GETLIST			0x00000012UL
#define IPMSG_ANSLIST			0x00000013UL
#define IPMSG_BR_ISGETLIST2		0x00000018UL

#define IPMSG_SENDMSG			0x00000020UL
#define IPMSG_RECVMSG			0x00000021UL
#define IPMSG_READMSG			0x00000030UL
#define IPMSG_DELMSG			0x00000031UL
#define IPMSG_ANSREADMSG		0x00000032UL

#define IPMSG_GETINFO			0x00000040UL
#define IPMSG_SENDINFO			0x00000041UL

#define IPMSG_GETABSENCEINFO	0x00000050UL
#define IPMSG_SENDABSENCEINFO	0x00000051UL

#define IPMSG_GETFILEDATA		0x00000060UL
#define IPMSG_RELEASEFILES		0x00000061UL
#define IPMSG_GETDIRFILES		0x00000062UL

#define IPMSG_GETPUBKEY			0x00000072UL
#define IPMSG_ANSPUBKEY			0x00000073UL

/* FeiQ "正在输入" / ShowInputStatus. Not in the IPMSG spec.
   Captured FeiQ datagrams use command 121 while the peer is typing and
   122 when typing pauses (low 8 bits; no option bits). The version field
   on those captures is FeiQ's "1_lbt…" token. We keep version "1" so a
   classic IPMSG parser still splits the packet and ignores an unknown
   mode instead of seeing a broken header. FeiQ may only paint the hint
   for its own version string — that needs a human check. */
#define IPMSG_FEIQ_TYPING		121UL
#define IPMSG_FEIQ_TYPINGSTOP		122UL

/* qipmsg private extension. Not a FeiQ opcode.
   UDP body "INSTANT:<packet>:<fileId>:<sha256>" finishes a send when the
   peer already has the file. UDP body "RICH:<packet>:<base64 utf-8 html>"
   is an optional formatting sidecar for the plain SENDMSG with that packet
   number. The SENDMSG body stays plain text so FeiQ still displays it.
   TCP body on the file port lists or downloads a qipmsg shared folder
   (see shareFolder). FeiQ ignores unknown modes; we do not send this to
   peers that did not advertise "qipmsg". */
#define IPMSG_QIPMSG_EXT		126UL

/*  option for all command  */
#define IPMSG_ABSENCEOPT		0x00000100UL
#define IPMSG_SERVEROPT			0x00000200UL
#define IPMSG_DIALUPOPT			0x00010000UL
#define IPMSG_FILEATTACHOPT		0x00200000UL
#define IPMSG_ENCRYPTOPT		0x00400000UL
/* IPMSG 3.x (ipmsg.h Ver4.50). UTF8OPT marks this datagram as UTF-8.
   CAPUTF8OPT / CLIPBOARDOPT are announce bits on BR_ENTRY / ANSENTRY.
   FeiQ 2013 ignores unknown option bits and keeps speaking GBK. */
#define IPMSG_UTF8OPT			0x00800000UL
#define IPMSG_CAPUTF8OPT		0x01000000UL
#define IPMSG_CLIPBOARDOPT		0x08000000UL

/*  option for send command  */
#define IPMSG_SENDCHECKOPT		0x00000100UL
#define IPMSG_SECRETOPT			0x00000200UL
#define IPMSG_BROADCASTOPT		0x00000400UL
#define IPMSG_MULTICASTOPT		0x00000800UL
#define IPMSG_NOPOPUPOPT		0x00001000UL
#define IPMSG_AUTORETOPT		0x00002000UL
#define IPMSG_RETRYOPT			0x00004000UL
#define IPMSG_PASSWORDOPT		0x00008000UL
#define IPMSG_NOLOGOPT			0x00020000UL
#define IPMSG_NEWMUTIOPT		0x00040000UL
#define IPMSG_NOADDLISTOPT		0x00080000UL
#define IPMSG_READCHECKOPT		0x00100000UL
#define IPMSG_SECRETEXOPT		(IPMSG_READCHECKOPT|IPMSG_SECRETOPT)

/* encryption flags for encrypt command */
#define IPMSG_RSA_512			0x00000001UL
#define IPMSG_RSA_1024			0x00000002UL
#define IPMSG_RSA_2048			0x00000004UL
#define IPMSG_RC2_40			0x00001000UL
#define IPMSG_RC2_128			0x00004000UL
#define IPMSG_RC2_256			0x00008000UL
#define IPMSG_BLOWFISH_128		0x00020000UL
#define IPMSG_BLOWFISH_256		0x00040000UL
/* Capability word inside GETPUBKEY / ANSPUBKEY / encrypted body.
   Not a command-option bit. 0x00800000 in that word is packet-number IV
   (ipmsg.h Ver4.50); the same numeric value on the command word is UTF8OPT. */
#define IPMSG_AES_256			0x00100000UL
#define IPMSG_SIGN_MD5			0x10000000UL

/* compatibilty for Win beta version */
#define IPMSG_RC2_40OLD			0x00000010UL	// for beta1-4 only
#define IPMSG_RC2_128OLD		0x00000040UL	// for beta1-4 only
#define IPMSG_BLOWFISH_128OLD	0x00000400UL	// for beta1-4 only
#define IPMSG_RC2_40ALL			(IPMSG_RC2_40|IPMSG_RC2_40OLD)
#define IPMSG_RC2_128ALL		(IPMSG_RC2_128|IPMSG_RC2_128OLD)
#define IPMSG_BLOWFISH_128ALL	(IPMSG_BLOWFISH_128|IPMSG_BLOWFISH_128OLD)

/* file types for fileattach command */
#define IPMSG_FILE_REGULAR		0x00000001UL
#define IPMSG_FILE_DIR			0x00000002UL
#define IPMSG_FILE_RETPARENT	0x00000003UL	// return parent directory
#define IPMSG_FILE_SYMLINK		0x00000004UL
#define IPMSG_FILE_CDEV			0x00000005UL	// for UNIX
#define IPMSG_FILE_BDEV			0x00000006UL	// for UNIX
#define IPMSG_FILE_FIFO			0x00000007UL	// for UNIX
#define IPMSG_FILE_RESFORK		0x00000010UL	// for Mac
#define IPMSG_FILE_CLIPBOARD		0x00000020UL	// Windows clipboard (IPMSG 3)

/* file attribute options for fileattach command */
#define IPMSG_FILE_RONLYOPT		0x00000100UL
#define IPMSG_FILE_HIDDENOPT	0x00001000UL
#define IPMSG_FILE_EXHIDDENOPT	0x00002000UL	// for MacOS X
#define IPMSG_FILE_ARCHIVEOPT	0x00004000UL
#define IPMSG_FILE_SYSTEMOPT	0x00008000UL

/* extend attribute types for fileattach command */
#define IPMSG_FILE_UID			0x00000001UL
#define IPMSG_FILE_USERNAME		0x00000002UL	// uid by string
#define IPMSG_FILE_GID			0x00000003UL
#define IPMSG_FILE_GROUPNAME	0x00000004UL	// gid by string
#define IPMSG_FILE_PERM			0x00000010UL	// for UNIX
#define IPMSG_FILE_MAJORNO		0x00000011UL	// for UNIX devfile
#define IPMSG_FILE_MINORNO		0x00000012UL	// for UNIX devfile
#define IPMSG_FILE_CTIME		0x00000013UL	// for UNIX
#define IPMSG_FILE_MTIME		0x00000014UL
#define IPMSG_FILE_ATIME		0x00000015UL
#define IPMSG_FILE_CREATETIME	0x00000016UL
#define IPMSG_FILE_CREATOR		0x00000020UL	// for Mac
#define IPMSG_FILE_FILETYPE		0x00000021UL	// for Mac
#define IPMSG_FILE_FINDERINFO	0x00000022UL	// for Mac
#define IPMSG_FILE_ACL			0x00000030UL
#define IPMSG_FILE_ALIASFNAME	0x00000040UL	// alias fname
#define IPMSG_FILE_UNICODEFNAME	0x00000041UL	// UNICODE fname
/* qipmsg-only extend-attr. SHA-256 hex of a regular file. Omitted unless
   the peer's presence token starts with "qipmsg", so FeiQ file lists stay
   the same shape they already accept (mtime / createtime). */
#define IPMSG_FILE_QIPHASH		0x00000071UL

#define FILELIST_SEPARATOR	'\a'
#define HOSTLIST_SEPARATOR	'\a'
#define HOSTLIST_DUMMY		"\b"

/*  end of IP Messenger Communication Protocol version 1.2 define  */

#endif // !IPMSG_H

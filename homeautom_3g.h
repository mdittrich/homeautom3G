/*
 * homeautom_3g.h
 *
 * Copyright Martin Dittrich
 * Hamburg University of Technology
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL I
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HOMEAUTOM_3G_H_
#define HOMEAUTOM_3G_H_

int readLine(unsigned int timeout)
void sendAT(const char *at_cmd, const char *secure_str)
int waitForReceive(const char *whitelist, const char *blacklist, unsigned int timeout)
int waitForRI(void)
void wakeUpByDTR(void)
void sendSMS(char *number, char *text)
void powerUD(void)
int hash(const char *str)
int count(char **arr)
char *getStringPart(const char *action_str, char delim, unsigned int first_occ, unsigned int sec_occ)
char **getSMSText(char *memory)

#endif /* HOMEAUTOM_3G_H_ */


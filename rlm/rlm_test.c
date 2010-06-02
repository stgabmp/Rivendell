/* rlm_test.c
 *
 *   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2
 *   as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This is a sample Rivendell Loadable Module.  All it does is print
 * Now & Next data to standard output for each event transition.
 *
 * To compile this module, just do:
 * 
 *   gcc -shared -o rlm_test.rlm rlm_test.c
 */

#include <stdio.h>
#include <string.h>
#include <rlm/rlm.h>


void rlm_test_RLMStart(void *ptr,const char *arg)
{
  char str[1024];
  sprintf(str,"rlm_test: started on %s",
	  RLMDateTime(ptr,0,"MM/dd/yyyy at hh:mm:ss"));
  RLMLog(ptr,LOG_NOTICE,str);
}


void rlm_test_RLMFree(void *ptr)
{
}


void rlm_test_RLMPadDataSent(void *ptr,const char *svcname,int onair,int lognum,
			     const struct rlm_pad *now,
			     const struct rlm_pad *next)
{
  printf("Service: %s\n",svcname);
  if(onair==0) {
    printf(" OnAir = false\n");
  }
  else {
    printf(" OnAir = true\n");
  }
  switch(lognum) {
    case 0:
      printf(" -- On Main Log ---------------------------------------------\n");
      break;

    case 1:
      printf(" -- On Aux 1 Log --------------------------------------------\n");
      break;

    case 2:
      printf(" -- On Aux 2 Log --------------------------------------------\n");
      break;
  }
  printf("Playing NOW\n");
  printf("  Cart number: %06u\n",now->rlm_cartnum);
  printf("       Length: %u mS\n",now->rlm_len);
  printf("        Title: %s\n",now->rlm_title);
  printf("       Artist: %s\n",now->rlm_artist);
  printf("        Label: %s\n",now->rlm_label);
  printf("       Client: %s\n",now->rlm_client);
  printf("       Agency: %s\n",now->rlm_agency);
  printf("     Composer: %s\n",now->rlm_comp);
  printf("    Publisher: %s\n",now->rlm_pub);
  printf("  UserDefined: %s\n",now->rlm_userdef);
  printf("\n");
  printf("Playing NEXT\n");
  printf("  Cart number: %06u\n",next->rlm_cartnum);
  printf("       Length: %u mS\n",next->rlm_len);
  printf("        Title: %s\n",next->rlm_title);
  printf("       Artist: %s\n",next->rlm_artist);
  printf("        Label: %s\n",next->rlm_label);
  printf("       Client: %s\n",next->rlm_client);
  printf("       Agency: %s\n",next->rlm_agency);
  printf("     Composer: %s\n",next->rlm_comp);
  printf("    Publisher: %s\n",next->rlm_pub);
  printf("  UserDefined: %s\n",next->rlm_userdef);
}

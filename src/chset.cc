/*
 * Copyright (c) 1993-2013 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "edit.h"
#include "keymap.h"
#include "mb.h"

byte  chset[CHSET_SIZE+1];

void  clear_chset()
{
   int i;
   for(i=0; i<CHSET_SIZE; i++)
      chset[i]='@';
   chset[i]=0;
}

void  set_chset_bit(int i)
{
   chset[i/CHSET_BITS_PER_BYTE]|=(1<<i%CHSET_BITS_PER_BYTE);
}

void  init_chset()
{
/*   int i;
   clear_chset();
   for(i=0; i<256; i++)
      if(iscntrl(i))
         chset[i/CHSET_BITS_PER_BYTE]|=(1<<i%CHSET_BITS_PER_BYTE);*/
   set_chset_8bit_noctrl();
}

void  set_chset_8bit()
{
   int i;
   clear_chset();
   for(i=0; i<32; i++)
      set_chset_bit(i);
   set_chset_bit(127);
}
void  set_chset_8bit_noctrl()
{
   int i;
   clear_chset();
   for(i=0; i<32; i++)
      set_chset_bit(i);
   for(i=127; i<128+32; i++)
      set_chset_bit(i);
}
void  set_chset_no8bit()
{
   int i;
   clear_chset();
   for(i=0; i<32; i++)
      set_chset_bit(i);
   for(i=127; i<256; i++)
      set_chset_bit(i);
}

void  edit_chset()
{
   WIN *w;
   int i,j;
   static int curr=0;
   byte  old[CHSET_SIZE];
   char  s[256];
   char  chstr[3];
   int    action;

   memcpy(old,chset,sizeof(old));
   w=CreateWin(MIDDLE,MIDDLE,3*16+4,16+6,NORMAL_TEXT_ATTR," Character Set Visualisation ",0);
   DisplayWin(w);

   for(;;)
   {
      SetAttr(NORMAL_TEXT_ATTR);
      Clear();
      if(curr<32)
         snprintf(chstr,sizeof(chstr),"^%c",curr+'@');
      else
      {
#if USE_MULTIBYTE_CHARS
	 if(mb_mode)
	 {
	    int w=wcwidth(curr);
	    if(w==0)
	       chstr[0]=' ';  // to show accents nicely.
	    int ch_len=wctomb(chstr+(w==0),curr);
	    if(ch_len<0)
	       ch_len=0;
	    chstr[ch_len+(w==0)]=0;
	 }
	 else // note the following line
#endif
	    snprintf(chstr,sizeof(chstr),"%c",curr);
      }
      snprintf(s,sizeof(s),"The current character is '%s', %3d, 0%03o, 0x%02X",chstr,curr,curr,curr);

      PutStr(2,2,s);
      for(i=0; i<16; i++)
         for(j=0; j<16; j++)
         {
            SetAttr((i<<4)+j==curr?CURR_BUTTON_ATTR:NORMAL_TEXT_ATTR);
            PutCh(i*3+2,j+4,' ');
            PutCh(i*3+3,j+4,(i<<4)+j);
            PutCh(i*3+4,j+4,' ');
         }
      action=GetNextAction();
      switch(action)
      {
      case(CANCEL):
         memcpy(chset,old,sizeof(old));
      case(NEWLINE):
         goto done;
      case(LINE_UP):
         if((curr&15)==0)
            curr|=15;
         else
            curr--;
         break;
      case(LINE_DOWN):
         if((curr&15)==15)
            curr&=~15;
         else
            curr++;
         break;
      case(CHAR_LEFT):
         if((curr&0xF0)==0)
            curr|=0xF0;
         else
            curr-=16;
         break;
      case(CHAR_RIGHT):
         if((curr&0xF0)==0xF0)
            curr&=~0xF0;
         else
            curr+=16;
         break;
      default:
         if(StringTypedLen!=1)
            break;
         switch(StringTyped[0])
         {
         case(' '):
            if(curr<32)
               chset[curr/CHSET_BITS_PER_BYTE]|=(1<<(curr%CHSET_BITS_PER_BYTE));
            else
               chset[curr/CHSET_BITS_PER_BYTE]^=(1<<(curr%CHSET_BITS_PER_BYTE));
            clearok(stdscr,TRUE);
            break;
         }
      }
   }
done:
   flag=REDISPLAY_ALL;
   CloseWin();
   DestroyWin(w);
}

int  choose_ch()
{
   WIN *w;
   int i,j;
   static int curr=0;
   int   res=-1;
   char  s[256];
   char  chstr[MB_CUR_MAX+2];
   int   action;

   w=CreateWin(MIDDLE,MIDDLE,3*16+4,16+6,DIALOGUE_WIN_ATTR," Character Set ",0);
   DisplayWin(w);

   for(;;)
   {
      SetAttr(DIALOGUE_WIN_ATTR);
      Clear();

      if(curr<32)
         snprintf(chstr,sizeof(chstr),"^%c",curr+'@');
      else
      {
#if USE_MULTIBYTE_CHARS
	 if(0&&mb_mode)
	 {
	    int w=wcwidth(curr);
	    if(w==0)
	       chstr[0]=' ';  // to show accents nicely.
	    int ch_len=wctomb(chstr+(w==0),curr);
	    if(ch_len<0)
	       ch_len=0;
	    chstr[ch_len+(w==0)]=0;
	 }
	 else // note the following line
#endif
	    snprintf(chstr,sizeof(chstr),"%c",curr);
      }
      snprintf(s,sizeof(s),"The current character is '%s', %3d, 0%03o, 0x%02X",chstr,curr,curr,curr);

      PutStr(2,2,s);
      for(i=0; i<16; i++)
         for(j=0; j<16; j++)
         {
            SetAttr((i<<4)+j==curr?CURR_BUTTON_ATTR:DIALOGUE_WIN_ATTR);
            PutCh(i*3+2,j+4,' ');
#ifdef USE_MULTIBYTE_CHARS
	    if(0&&mb_mode)
	       PutWCh(i*3+3,j+4,(i<<4)+j);
	    else // note the next line
#endif
	       PutCh(i*3+3,j+4,(i<<4)+j);
            PutCh(i*3+4,j+4,' ');
         }
      action=GetNextAction();
      switch(action)
      {
      case(NEWLINE):
         res=curr;
         goto done;
      case(CANCEL):
         res=-1;
         goto done;
      case(LINE_UP):
         if((curr&15)==0)
            curr|=15;
         else
            curr--;
         break;
      case(LINE_DOWN):
         if((curr&15)==15)
            curr&=~15;
         else
            curr++;
         break;
      case(CHAR_LEFT):
         if((curr&0xF0)==0)
            curr|=0xF0;
         else
            curr-=16;
         break;
      case(CHAR_RIGHT):
         if((curr&0xF0)==0xF0)
            curr&=~0xF0;
         else
            curr+=16;
         break;
      }
   }
done:
   CloseWin();
   DestroyWin(w);
   return(res);
}
#if USE_MULTIBYTE_CHARS
wchar_t choose_wch()
{
   WIN *w;
   int i,j;
   static wchar_t curr=0;
   wchar_t res=-1;
   char  s[256];
   char  chstr[MB_CUR_MAX+2];
   int   action;

   w=CreateWin(MIDDLE,MIDDLE,3*16+4,16+6,DIALOGUE_WIN_ATTR," Character Set ",0);
   DisplayWin(w);

   for(;;)
   {
      SetAttr(DIALOGUE_WIN_ATTR);
      Clear();

      if(curr/256)
	 PutStr(FRIGHT-3,FDOWN," PgUp/PgDn ");
      else if(mb_mode)
	 PutStr(FRIGHT-6,FDOWN," PgDn ");

      if(curr<32)
         snprintf(chstr,sizeof(chstr),"^%c",curr+'@');
      else
      {
	 int w=wcwidth(curr);
	 if(w==0)
	    chstr[0]=' ';  // to show accents nicely.
	 int ch_len=wctomb(chstr+(w==0),curr);
	 if(ch_len<0)
	    ch_len=0;
	 chstr[ch_len+(w==0)]=0;
      }
      snprintf(s,sizeof(s),"The current character is '%s', 0x%04X",chstr,curr);

      PutStr(2,2,s);
      for(i=0; i<16; i++)
	 for(j=0; j<16; j++)
         {
            SetAttr((i<<4)+j==curr%256?CURR_BUTTON_ATTR:DIALOGUE_WIN_ATTR);
            PutCh(i*3+2,j+4,' ');
            PutWCh(i*3+3,j+4,(i<<4)+j+(curr&~255));
            PutCh(i*3+4,j+4,' ');
         }

      action=GetNextAction();
      switch(action)
      {
      case(NEWLINE):
         res=curr;
         goto done;
      case(CANCEL):
         res=-1;
         goto done;
      case(LINE_UP):
         if((curr&15)==0)
            curr|=15;
         else
            curr--;
         break;
      case(LINE_DOWN):
         if((curr&15)==15)
            curr&=~15;
         else
            curr++;
         break;
      case(CHAR_LEFT):
         if((curr&0xF0)==0)
            curr|=0xF0;
         else
            curr-=16;
         break;
      case(CHAR_RIGHT):
         if((curr&0xF0)==0xF0)
            curr&=~0xF0;
         else
            curr+=16;
         break;
      case(NEXT_PAGE):
	 curr+=256;
	 break;
      case(PREV_PAGE):
	 if(curr<256)
	    break;
	 curr-=256;
	 break;
      }
   }
done:
   CloseWin();
   DestroyWin(w);
   return(res);
}
#endif

void  addch_visual(chtype ch)
{
   attrset(curr_attr->n_attr);
   if(ch&A_ALTCHARSET)
      addch(ch);
   else
   {
      unsigned char ct=ch&A_CHARTEXT;
      if(!chset_isprint(ct))
      {
	 if(ct<32)
	    ct+='@';
	 else if(ct==127)
	    ct='?';
	 else
	    ct='.';
	 attrset(curr_attr->so_attr);
	 addch(ct);
	 attrset(curr_attr->n_attr);
      }
      else
	 addch(ch);
   }
}

bool chset_isprint(int c)
{
   if(c>=256 || c<0)
      return true;
   return !(chset[c/CHSET_BITS_PER_BYTE]&(1<<(c%CHSET_BITS_PER_BYTE)));
}

chtype visualize(const attr *a,chtype ch)
{
   unsigned char ct=ch&A_CHARTEXT;
   if(!chset_isprint(ct))
   {
      if(ct<32)
	 ct+='@';
      else if(ct==127)
	 ct='?';
      else
	 ct='.';
      return ct|a->so_attr;
   }
   return ch;
}

#if USE_MULTIBYTE_CHARS
static wchar_t visualize_wchar_nocache(wchar_t wc)
{
   unsigned char mbch[MB_CUR_MAX];
   if (wctomb(0, 0) < 0)
      /*ignore*/;
   int mbch_len=wctomb((char*)mbch,wc);
   if(mbch_len==1 && !chset_isprint(mbch[0]))
   {
      if(mbch[0]<32)
	 return mbch[0]+'@';
      else if(mbch[0]==127)
	 return '?';
      else
	 return '.';
   }
   if(wc>=256)
      return wc;
   if(wc>=0 && wc<256 && chset_isprint(wc))
      return wc;
   if(wc==0x80 && !chset_isprint(wc))
      return '.';
   if(wc<32)
      wc+='@';
   else if(wc==127)
      wc='?';
   else
      wc='.';
   return wc;
}

static wchar_t **vis_cache;
static int vis_cache_len;
static const int vis_cache_per_row=256;
static inline bool cache_row(const int row)
{
   if(__builtin_expect(row>=vis_cache_len,false)) {
      int new_vis_cache_len=row+1;
      wchar_t **new_vis_cache=(wchar_t**)realloc(vis_cache,new_vis_cache_len*sizeof(*vis_cache));
      if(!new_vis_cache)
	 return false;
      // clear new storage
      while(vis_cache_len<new_vis_cache_len)
	 new_vis_cache[vis_cache_len++]=NULL;
      vis_cache=new_vis_cache;
   }

   if(__builtin_expect(!vis_cache[row],false)) {
      vis_cache[row]=(wchar_t*)malloc(vis_cache_per_row*sizeof(wchar_t));
      if(!vis_cache[row])
	 return false;
      wchar_t wc=row*vis_cache_per_row;
      for(int i=0; i<vis_cache_per_row; i++)
	 vis_cache[row][i]=visualize_wchar_nocache(wc+i);
   }

   return true;
}

wchar_t visualize_wchar(wchar_t wc)
{
   if(wc<0)
      return wc;

   int row=wc/vis_cache_per_row;
   int col=wc%vis_cache_per_row;

   if(__builtin_expect(cache_row(row),true))
      return vis_cache[row][col];

   return visualize_wchar_nocache(wc);
}

#endif //USE_MULTIBYTE_CHARS

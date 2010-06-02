// schedcartlist.cpp
//
// A class for handling carts to be used in scheduler
//
//   Stefan Gabriel <stg@st-gabriel.de>
//
//   
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <schedcartlist.h>
#include <cstdlib>


SchedCartList::SchedCartList()
{
	itemcounter=0;
}

SchedCartList::~SchedCartList()
{
}

void SchedCartList::insertItem(unsigned cartnumber,int cartlength,int stack_id,QString stack_artist,QString stack_schedcodes)
{
	cartnum.push_back(cartnumber);
	cartlen.push_back(cartlength);
	stackid.push_back(stack_id);
	artist.push_back(stack_artist.lower().replace(" ",""));
        sched_codes.push_back(stack_schedcodes);
	valid.push_back(true);	
	itemcounter++;
}


void SchedCartList::removeItem(int itemnumber)
{
   if(valid[itemnumber]) {
	valid[itemnumber]=false;		
	itemcounter--;
    }
}

bool SchedCartList::removeIfCode(int itemnumber,QString test_code)
{
    QString test = test_code.leftJustify(11,' ',TRUE);

    if (sched_codes[itemnumber].find(test)!=-1 && valid[itemnumber])
      {
	valid[itemnumber]=false;		
	itemcounter--;
        return true;
      }
    return false; 
}

bool SchedCartList::itemHasCode(int itemnumber,QString test_code)
{
    QString test=test_code.leftJustify(11,' ',TRUE);

    if (sched_codes[itemnumber].find(test)!=-1)
      return true;
    else
      return false;
}


void SchedCartList::save(void)
{
	saveitemcounter=itemcounter;	
	savecartnum=cartnum;
	savecartlen=cartlen;
	savestackid=stackid;
	saveartist=artist;
	save_sched_codes=sched_codes;
	save_valid=valid;
}


void SchedCartList::restore(void)
{
	if(itemcounter==0)
	{
		cartnum=savecartnum;
		cartlen=savecartlen;
		stackid=savestackid;
		artist=saveartist;
		sched_codes=save_sched_codes;
		itemcounter=saveitemcounter;	
		valid=save_valid;
	}
}



unsigned SchedCartList::getItemCartnumber(int itemnumber)
{
	return cartnum[itemnumber];
}

int SchedCartList::getItemStackid(int itemnumber)
{
	return stackid[itemnumber];
}

QString SchedCartList::getItemArtist(int itemnumber)
{
	return artist[itemnumber];
}

QString SchedCartList::getItemSchedCodes(int itemnumber)
{
	return sched_codes[itemnumber];
}

int SchedCartList::getItemCartlength(int itemnumber)
{
	return cartlen[itemnumber];
}


int SchedCartList::getNumberOfItems(void)
{
	return itemcounter;
}

 
int SchedCartList::getTotalNumberOfItems(void)
{
	return cartnum.size();
}
 

int SchedCartList::getRandom(void)
{
	if(itemcounter<1) {
	  return 0;
	}
        if(cartnum.size()==1) {
          return 0;
        }
        unsigned pos=rand()%(cartnum.size()-1);
	while(valid[pos]==false) {
	  pos++;
	  if(pos>=cartnum.size()) {
	    pos=0;
	  }
	}
	return pos;	
}
 

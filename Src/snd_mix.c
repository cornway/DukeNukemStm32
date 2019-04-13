/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_mix.c -- portable code to mix sounds for snd_dma.c

#include "sounds.h"

#ifdef _WIN32
#include "winquake.h"
#else
#define DWORD	unsigned long
#endif

extern volatile dma_t  *shm;

#define	PAINTBUFFER_SIZE	512
portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
int		snd_scaletable[32][256];
int 	*snd_p, snd_linear_count, snd_vol;
short	*snd_out;

void Snd_WriteLinearBlastStereo16 (void);

#if	!id386
void Snd_WriteLinearBlastStereo16 (void)
{
	int		i;
	int		val;

	for (i=0 ; i<snd_linear_count ; i+=2)
	{
		val = (snd_p[i]*snd_vol)>>8;
		if (val > 0x7fff)
			snd_out[i] = 0x7fff;
		else if (val < (short)0x8000)
			snd_out[i] = (short)0x8000;
		else
			snd_out[i] = val;

		val = (snd_p[i+1]*snd_vol)>>8;
		if (val > 0x7fff)
			snd_out[i+1] = 0x7fff;
		else if (val < (short)0x8000)
			snd_out[i+1] = (short)0x8000;
		else
			snd_out[i+1] = val;
	}
}
#endif

void S_TransferStereo16 (int endtime)
{
	int		lpos;
	int		lpaintedtime;
	DWORD	*pbuf;
#ifdef _WIN32
	int		reps;
	DWORD	dwSize,dwSize2;
	DWORD	*pbuf2;
	HRESULT	hresult;
#endif
	
	snd_vol = 80256;

	snd_p = (int *) paintbuffer;
	lpaintedtime = 0;

#ifdef _WIN32
	if (pDSBuf)
	{
		reps = 0;

		while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, &pbuf, &dwSize, 
									   &pbuf2, &dwSize2, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("S_TransferStereo16: DS::Lock Sound Buffer Failed\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("S_TransferStereo16: DS: couldn't restore buffer\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}
		}
	}
	else
#endif
	{
		pbuf = (DWORD *)shm->buffer;
	}

	while (lpaintedtime < endtime)
	{
	// handle recirculating buffer issues
		lpos = lpaintedtime & ((shm->samples>>1)-1);

		snd_out = (short *) pbuf + (lpos<<1);

		snd_linear_count = (shm->samples>>1) - lpos;
		if (lpaintedtime + snd_linear_count > endtime)
			snd_linear_count = endtime - lpaintedtime;

		snd_linear_count <<= 1;

	// write a linear blast of samples
		Snd_WriteLinearBlastStereo16 ();

		snd_p += snd_linear_count;
		lpaintedtime += (snd_linear_count>>1);
	}

#ifdef _WIN32
	if (pDSBuf)
		pDSBuf->lpVtbl->Unlock(pDSBuf, pbuf, dwSize, NULL, 0);
#endif
}

void S_TransferPaintBuffer(int endtime)
{
	int 	out_idx;
	int 	count;
	int 	out_mask;
	int 	*p;
	int 	step;
	int		val;
	int		snd_vol;
	DWORD	*pbuf;
#ifdef _WIN32
	int		reps;
	DWORD	dwSize,dwSize2;
	DWORD	*pbuf2;
	HRESULT	hresult;
#endif

	if (shm->samplebits == 16 && shm->channels == 2)
	{
		S_TransferStereo16 (endtime);
		return;
	}
	
	p = (int *) paintbuffer;
	count = (endtime - 0) * shm->channels;
	out_mask = shm->samples - 1; 
	out_idx = 0 * shm->channels & out_mask;
	step = 3 - shm->channels;
	snd_vol = 80*256;

#ifdef _WIN32
	if (pDSBuf)
	{
		reps = 0;

		while ((hresult = pDSBuf->lpVtbl->Lock(pDSBuf, 0, gSndBufSize, &pbuf, &dwSize, 
									   &pbuf2,&dwSize2, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("S_TransferPaintBuffer: DS::Lock Sound Buffer Failed\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("S_TransferPaintBuffer: DS: couldn't restore buffer\n");
				S_Shutdown ();
				S_Startup ();
				return;
			}
		}
	}
	else
#endif
	{
		pbuf = (DWORD *)shm->buffer;
	}

	if (shm->samplebits == 16)
	{
		short *out = (short *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = val;
			out_idx = (out_idx + 1) & out_mask;
		}
	}
	else if (shm->samplebits == 8)
	{
		unsigned char *out = (unsigned char *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = (val>>8) + 128;
			out_idx = (out_idx + 1) & out_mask;
		}
	}

#ifdef _WIN32
	if (pDSBuf) {
		DWORD dwNewpos, dwWrite;
		int il = paintedtime;
		int ir = endtime - paintedtime;
		
		ir += il;

		pDSBuf->lpVtbl->Unlock(pDSBuf, pbuf, dwSize, NULL, 0);

		pDSBuf->lpVtbl->GetCurrentPosition(pDSBuf, &dwNewpos, &dwWrite);

//		if ((dwNewpos >= il) && (dwNewpos <= ir))
//			Con_Printf("%d-%d p %d c\n", il, ir, dwNewpos);
	}
#endif
}


/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

void SND_InitScaletable (void)
{
	int		i, j;
	
	for (i=0 ; i<32 ; i++)
		for (j=0 ; j<256 ; j++)
			snd_scaletable[i][j] = ((signed char)j) * i * 8;
}


#if	!id386

#endif	// !id386



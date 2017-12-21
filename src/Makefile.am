SUBDIRS = enet imgfun audio platform ui
if USE_CURL
SUBDIRS += curl
endif

bin_PROGRAMS = 7kaa
7kaa_SOURCES = \
	AM.cpp \
	OAI_ACT.cpp \
	OAI_ACT2.cpp \
	OAI_ATTK.cpp \
	OAI_BUIL.cpp \
	OAI_CAP2.cpp \
	OAI_CAPT.cpp \
	OAI_DEFE.cpp \
	OAI_DIPL.cpp \
	OAI_ECO.cpp \
	OAI_GRAN.cpp \
	OAI_INFO.cpp \
	OAI_MAIN.cpp \
	OAI_MAR2.cpp \
	OAI_MAR3.cpp \
	OAI_MARI.cpp \
	OAI_MILI.cpp \
	OAI_MONS.cpp \
	OAI_QUER.cpp \
	OAI_SEEK.cpp \
	OAI_SPY.cpp \
	OAI_TALK.cpp \
	OAI_TOWN.cpp \
	OAI_TRAD.cpp \
	OAI_UNIT.cpp \
	OANLINE.cpp \
	OBATTLE.cpp \
	OBLOB.cpp \
	OBOX.cpp \
	OBULLET.cpp \
	OBULLETA.cpp \
	OBUTT3D.cpp \
	OBUTTCUS.cpp \
	OBUTTON.cpp \
	OB_FLAME.cpp \
	OB_HOMIN.cpp \
	OB_PROJ.cpp \
	OCOLTBL.cpp \
	OCONFIG.cpp \
	OCRC_STO.cpp \
	ODATE.cpp \
	ODB.cpp \
	ODIR.cpp \
	ODYNARR.cpp \
	ODYNARRB.cpp \
	OEFFECT.cpp \
	OERRCTRL.cpp \
	OERROR.cpp \
	OEXPMASK.cpp \
	OFILE.cpp \
	OFILETXT.cpp \
	OFIRM.cpp \
	OFIRM2.cpp \
	OFIRMA.cpp \
	OFIRMAI.cpp \
	OFIRMDIE.cpp \
	OFIRMDRW.cpp \
	OFIRMIF.cpp \
	OFIRMIF2.cpp \
	OFIRMIF3.cpp \
	OFIRMRES.cpp \
	OFLAME.cpp \
	OFONT.cpp \
	OF_BASE.cpp \
	OF_BASE2.cpp \
	OF_CAMP.cpp \
	OF_CAMP2.cpp \
	OF_FACT.cpp \
	OF_FACT2.cpp \
	OF_HARB.cpp \
	OF_HARB2.cpp \
	OF_INN.cpp \
	OF_INN2.cpp \
	OF_MARK.cpp \
	OF_MARK2.cpp \
	OF_MINE.cpp \
	OF_MINE2.cpp \
	OF_MONS.cpp \
	OF_RESE.cpp \
	OF_RESE2.cpp \
	OF_WAR.cpp \
	OF_WAR2.cpp \
	OGAMCRED.cpp \
	OGAME.cpp \
	OGAMEMP.cpp \
	OGAMENCY.cpp \
	OGAMEND.cpp \
	OGAMESET.cpp \
	OGAMHALL.cpp \
	OGAMMAIN.cpp \
	OGAMMENU.cpp \
	OGAMSCE2.cpp \
	OGAMSCEN.cpp \
	OGAMSING.cpp \
	OGENHILL.cpp \
	OGENMAP.cpp \
	OGETA.cpp \
	OGFILE.cpp \
	OGFILE2.cpp \
	OGFILE3.cpp \
	OGF_V1.cpp \
	OGODRES.cpp \
	OGRPSEL.cpp \
	OHELP.cpp \
	OHILLRES.cpp \
	OIMGRES.cpp \
	OINFO.cpp \
	OINGMENU.cpp \
	OLIGHTN.cpp \
	OLIGHTN2.cpp \
	OLOG.cpp \
	OLONGLOG.cpp \
	OLZW.cpp \
	OMATRIX.cpp \
	OMEM.cpp \
	OMISC.cpp \
	OMONSRES.cpp \
	OMOUSECR.cpp \
	OMP_CRC.cpp \
	OMUSIC.cpp \
	ONATIONA.cpp \
	ONATIONB.cpp \
	ONEWS.cpp \
	ONEWS2.cpp \
	ONEWSENG.cpp \
	OOPTMENU.cpp \
	OPLANT.cpp \
	OPLASMA.cpp \
	OPOWER.cpp \
	ORACERES.cpp \
	ORAIN1.cpp \
	ORAIN2.cpp \
	ORAIN3.cpp \
	ORAWRES.cpp \
	OREBEL.cpp \
	OREGION.cpp \
	OREGIONS.cpp \
	OREMOTE.cpp \
	OREMOTE2.cpp \
	OREMOTEM.cpp \
	OREMOTEQ.cpp \
	ORES.cpp \
	ORESDB.cpp \
	ORESX.cpp \
	OROCK.cpp \
	OROCKRES.cpp \
	OR_AI.cpp \
	OR_ECO.cpp \
	OR_MIL.cpp \
	OR_NAT.cpp \
	OR_NEWS.cpp \
	OR_RANK.cpp \
	OR_SPY.cpp \
	OR_TECH.cpp \
	OR_TOWN.cpp \
	OR_TRADE.cpp \
	OSaveGameArray.cpp \
	OSaveGameInfo.cpp \
	OSaveGameProvider.cpp \
	OSCROLL.cpp \
	OSE.cpp \
	OSERES.cpp \
	OSFRMRES.cpp \
	OSITE.cpp \
	OSITEDRW.cpp \
	OSKILL.cpp \
	OSLIDCUS.cpp \
	OSNOW1.cpp \
	OSNOW2.cpp \
	OSNOWG.cpp \
	OSNOWRES.cpp \
	OSPATH.cpp \
	OSPATHBT.cpp \
	OSPREDBG.cpp \
	OSPREOFF.cpp \
	OSPRESMO.cpp \
	OSPREUSE.cpp \
	OSPRITE.cpp \
	OSPRITE2.cpp \
	OSPRITEA.cpp \
	OSPRTRES.cpp \
	OSPY.cpp \
	OSPY2.cpp \
	OSPYA.cpp \
	OSTR.cpp \
	OSYS.cpp \
	OSYS2.cpp \
	OSYS3.cpp \
	OTALKENG.cpp \
	OTALKFRE.cpp \
	OTALKGER.cpp \
	OTALKMSG.cpp \
	OTALKRES.cpp \
	OTALKSPA.cpp \
	OTECHRES.cpp \
	OTERRAIN.cpp \
	OTORNADO.cpp \
	OTOWN.cpp \
	OTOWNA.cpp \
	OTOWNAI.cpp \
	OTOWNBLD.cpp \
	OTOWNDRW.cpp \
	OTOWNIF.cpp \
	OTOWNIND.cpp \
	OTOWNRES.cpp \
	OTUTOR.cpp \
	OTUTOR2.cpp \
	OTownNetwork.cpp \
	OUNIT.cpp \
	OUNIT2.cpp \
	OUNITA.cpp \
	OUNITAAC.cpp \
	OUNITAAT.cpp \
	OUNITAC.cpp \
	OUNITAI.cpp \
	OUNITAM.cpp \
	OUNITAMT.cpp \
	OUNITAT.cpp \
	OUNITAT2.cpp \
	OUNITAT3.cpp \
	OUNITATB.cpp \
	OUNITD.cpp \
	OUNITDRW.cpp \
	OUNITHB.cpp \
	OUNITI.cpp \
	OUNITIF.cpp \
	OUNITIND.cpp \
	OUNITM.cpp \
	OUNITRES.cpp \
	OUNITS.cpp \
	OU_CARA.cpp \
	OU_CARA2.cpp \
	OU_CARAS.cpp \
	OU_CARAT.cpp \
	OU_CART.cpp \
	OU_GOD.cpp \
	OU_GOD2.cpp \
	OU_MARI.cpp \
	OU_MARI2.cpp \
	OU_MARIF.cpp \
	OU_MARIS.cpp \
	OU_MARIT.cpp \
	OU_MONS.cpp \
	OU_VEHI.cpp \
	OVBROWIF.cpp \
	OVBROWSE.cpp \
	OVGABUF.cpp \
	OVGABUF2.cpp \
	OVGALOCK.cpp \
	OVIDEO.cpp \
	OVOLUME.cpp \
	OVQUEUE.cpp \
	OWALLRES.cpp \
	OWARPT.cpp \
	OWEATHER.cpp \
	OWORLD.cpp \
	OWORLD_M.cpp \
	OWORLD_Z.cpp \
	OW_FIRE.cpp \
	OW_PLANT.cpp \
	OW_ROCK.cpp \
	OW_SOUND.cpp \
	OW_WALL.cpp \
	ReplayFile.cpp \
	dbglog.cpp \
	file_input_stream.cpp \
	file_output_stream.cpp \
	file_reader.cpp \
	file_util.cpp \
	file_writer.cpp \
	FileSystem.cpp \
	input_stream.cpp \
	mem_input_stream.cpp \
	misc_uuid.cpp \
	output_stream.cpp \
	player_desc.cpp \
	vgautil.cpp \
	vgautil2.cpp


7kaa_LDADD =

if UI_SDL
7kaa_LDADD += ./ui/sdl/libui_sdl.a
endif
if UI_NONE
7kaa_LDADD += ./ui/none/libui_none.a
endif

if AUDIO_OPENAL
7kaa_LDADD += ./audio/openal/libaudio_openal.a
endif
if AUDIO_NONE
7kaa_LDADD += ./audio/none/libaudio_none.a
endif


7kaa_LDADD += ./enet/libmultiplayer.a


if USE_ASM
7kaa_LDADD += ./imgfun/x86/libimgfun_x86.a
else
7kaa_LDADD += ./imgfun/generic/libimgfun_generic.a
endif

7kaa_LDADD += ./platform/libplatform.a

7kaa_LDADD += @LIBINTL@

if USE_CURL
7kaa_LDADD += ./curl/libWebService.a
endif

if USE_WINDOWS
7kaa_SOURCES += ico.rc
SUFFIXES = .rc
.rc.o:
	windres -I "$(top_srcdir)/include" -i $< $@
ico.o: $(top_srcdir)/include/version.h 7k.ico
endif

EXTRA_DIST = 7k.ico ico.rc

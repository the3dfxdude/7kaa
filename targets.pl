my @obj_files;

### Do asm build ###
push(@obj_files, include_targets('asm/targets.pl'));
### End asm build ###

### Compile game ###
@targets = qw(
AM.cpp
ODPLAY.cpp
OGAMESET.cpp
ORAIN2.cpp
OSPRITE2.cpp
OUNITIF.cpp
OAI_ACT.cpp
ODYNARR.cpp
OGAMHALL.cpp
ORAIN3.cpp
OSPRITEA.cpp
OUNITIND.cpp
OAI_ACT2.cpp
ODYNARRB.cpp
OGAMMAIN.cpp
ORAWRES.cpp
OSPRTRES.cpp
OUNITM.cpp
OAI_ATTK.cpp
OEFFECT.cpp
OGAMMENU.cpp
OREBEL.cpp
OSPY.cpp
OUNITRES.cpp
OAI_BUIL.cpp
OERRCTRL.cpp
OGAMSCE2.cpp
OREGION.cpp
OSPY2.cpp
OUNITS.cpp
OAI_CAP2.cpp
OERROR.cpp
OGAMSCEN.cpp
OREGIONS.cpp
OSPYA.cpp
OU_CARA.cpp
OAI_CAPT.cpp
OEXPMASK.cpp
OGAMSING.cpp
OREMOTE.cpp
OSTR.cpp
OU_CARA2.cpp
OAI_DEFE.cpp
OFILE.cpp
OGENHILL.cpp
OREMOTE2.cpp
OSYS.cpp
OU_CARAS.cpp
OAI_DIPL.cpp
OFILETXT.cpp
OGENMAP.cpp
OREMOTEM.cpp
OSYS2.cpp
OU_CARAT.cpp
OAI_ECO.cpp
OFIRM.cpp
OGETA.cpp
OREMOTEQ.cpp
OSYS3.cpp
OU_CART.cpp
OAI_GRAN.cpp
OFIRM2.cpp
OGFILE.cpp
ORES.cpp
OTALKENG.cpp
OU_GOD.cpp
OAI_INFO.cpp
OFIRMA.cpp
OGFILE2.cpp
ORESDB.cpp
OTALKMSG.cpp
OU_GOD2.cpp
OAI_MAIN.cpp
OFIRMAI.cpp
OGFILE3.cpp
ORESX.cpp
OTALKRES.cpp
OU_MARI.cpp
OAI_MAR2.cpp
OFIRMDIE.cpp
OGFILEA.cpp
OROCK.cpp
OTECHRES.cpp
OU_MARI2.cpp
OAI_MAR3.cpp
OFIRMDRW.cpp
OGODRES.cpp
OROCKRES.cpp
OTERRAIN.cpp
OU_MARIF.cpp
OAI_MARI.cpp
OFIRMIF.cpp
OGRPSEL.cpp
OR_AI.cpp
OTORNADO.cpp
OU_MARIS.cpp
OAI_MILI.cpp
OFIRMIF2.cpp
OHELP.cpp
OR_ECO.cpp
OTOWN.cpp
OU_MARIT.cpp
OAI_MONS.cpp
OFIRMIF3.cpp
OHILLRES.cpp
OR_MIL.cpp
OTOWNA.cpp
OU_MONS.cpp
OAI_QUER.cpp
OFIRMRES.cpp
OIMGRES.cpp
OR_NAT.cpp
OTOWNAI.cpp
OU_VEHI.cpp
OAI_SEEK.cpp
OFONT.cpp
OINFO.cpp
OR_NEWS.cpp
OTOWNBLD.cpp
OVBROWIF.cpp
OAI_SPY.cpp
OF_BASE.cpp
OINGMENU.cpp
OR_RANK.cpp
OTOWNDRW.cpp
OVBROWSE.cpp
OAI_TALK.cpp
OF_BASE2.cpp
OLIGHTN.cpp
OR_SPY.cpp
OTOWNIF.cpp
OVGA.cpp
OAI_TOWN.cpp
OF_CAMP.cpp
OLIGHTN2.cpp
OR_TECH.cpp
OTOWNIND.cpp
OVGA2.cpp
OAI_TRAD.cpp
OF_CAMP2.cpp
OLOG.cpp
OR_TOWN.cpp
OTOWNRES.cpp
OVGABUF.cpp
OAI_UNIT.cpp
OF_FACT.cpp
OLZW.cpp
OR_TRADE.cpp
OTRANSL.cpp
OVGABUF2.cpp
OANLINE.cpp
OF_FACT2.cpp
OMATRIX.cpp
OSCROLL.cpp
OTUTOR.cpp
OVGALOCK.cpp
OAUDIO.cpp
OF_HARB.cpp
OMEM.cpp
OSE.cpp
OTUTOR2.cpp
OVIDEO.cpp
OBATTLE.cpp
OF_HARB2.cpp
OMISC.cpp
OSERES.cpp
OUNIT.cpp
OVOLUME.cpp
OBLOB.cpp
OF_INN.cpp
OMONSRES.cpp
OSFRMRES.cpp
OUNIT2.cpp
OVQUEUE.cpp
OBOX.cpp
OF_INN2.cpp
OMOUSE.cpp
OSITE.cpp
OUNITA.cpp
OWALLRES.cpp
OBULLET.cpp
OF_MARK.cpp
OMOUSECR.cpp
OSITEDRW.cpp
OUNITAAC.cpp
OWARPT.cpp
OBULLETA.cpp
OF_MARK2.cpp
OMP_CRC.cpp
OSKILL.cpp
OUNITAAT.cpp
OWEATHER.cpp
OBUTT3D.cpp
OF_MINE.cpp
OMUSIC.cpp
OSLIDCUS.cpp
OUNITAC.cpp
OWORLD.cpp
OBUTTCUS.cpp
OF_MINE2.cpp
ONATIONA.cpp
OSNOW1.cpp
OUNITAI.cpp
OWORLD_M.cpp
OBUTTON.cpp
OF_MONS.cpp
ONATIONB.cpp
OSNOW2.cpp
OUNITAM.cpp
OWORLD_Z.cpp
OB_FLAME.cpp
OF_RESE.cpp
ONEWS.cpp
OSNOWG.cpp
OUNITAMT.cpp
OW_FIRE.cpp
OB_HOMIN.cpp
OF_RESE2.cpp
ONEWS2.cpp
OSNOWRES.cpp
OUNITAT.cpp
OW_PLANT.cpp
OB_PROJ.cpp
OF_WAR.cpp
ONEWSENG.cpp
OSPATH.cpp
OUNITAT2.cpp
OW_ROCK.cpp
OCOLTBL.cpp
OF_WAR2.cpp
OOPTMENU.cpp
OSPATHBT.cpp
OUNITAT3.cpp
OW_SOUND.cpp
OCONFIG.cpp
OGAMCRED.cpp
OPLANT.cpp
OSPATHS2.cpp
OUNITATB.cpp
OW_WALL.cpp
OCRC_STO.cpp
OGAME.cpp
OPLASMA.cpp
OSPREOFF.cpp
OUNITD.cpp
OFLAME.cpp
ODATE.cpp
OGAMEMP.cpp
OPOWER.cpp
OSPRESMO.cpp
OUNITDRW.cpp
OGF_V1.cpp
ODB.cpp
OGAMENCY.cpp
ORACERES.cpp
OSPREUSE.cpp
OUNITHB.cpp
OMOUSESP.cpp
ODIR.cpp
OGAMEND.cpp
ORAIN1.cpp
OSPRITE.cpp
OUNITI.cpp
OMOUSEGE.cpp
OMOUSEFR.cpp
OTALKSPA.cpp
OTALKFRE.cpp
OTALKGER.cpp
ONEWSFRE.cpp
ONEWSSPA.cpp
ONEWSGER.cpp
OSPREDBG.cpp
OLONGLOG.cpp
ico.rc
);

@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}

@includes = qw( include );
 
if (defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}

if (defined($dxsdk_path)) {
  push (@includes, "$dxsdk_path/include");
}

push(@obj_files, build_targets(\@targets, \@includes, \@defines));
### End compile game ###

### Linking targets ###
@libs = qw(
  gdi32 ddraw msvcrt ole32 dinput dplayx dsound winmm
);

if (defined($dxsdk_path)) {
  push (@lib_dirs, "$dxsdk_path/lib");
}

link_exe ('7kaa.exe', \@obj_files, \@libs, \@lib_dirs);
### End linking ###

#!/bin/sh
#
# List the symbol info from elf file by order (c) 2015 WangXi
#
#
# Revision:
#	2015-3-10:Basic parser


# default configure
#elf_file_path="ntkdriver.ko"
#cross_toolchain_prefix="/home/xa00107/tool/tizen_gcc/armv7l-tizen-linux-gnueabi-"
elf_file_path=
cross_toolchain_prefix=

# grep command filter in
elf_section_filter_option=
elf_section_list=
elf_debug_info_check="no"
output_sort_reverse="no"
fix_syminfo_by_addr2line="no"
strip_path_levl=6

## parse option
for opt do
  optarg=`expr "x$opt" : 'x[^=]*=\(.*\)'`
  case "$opt" in
  --help|-h) show_help=yes
  ;;
  --version|-V)
    echo "elf-sort@v1.1"
    exit 0
  ;;
  --order-reverse)output_sort_reverse="yes"
  ;;
  --cross-prefix=*) cross_toolchain_prefix="$optarg"
  ;;
  --diff-with=*) diff_with_old_report="$optarg"
  ;;
  --prefix-strip=*) opt_num_val=`expr "$optarg" : '\([0-9]*\)'`
  strip_path_levl=${opt_num_val}
  ;;
  --elf=*) elf_file_path="$optarg"
  ;;
  --skip-line)
  elf_debug_info_check="no"
  ;;
  --fix-line)
  fix_syminfo_by_addr2line="yes";;
  --section=*)
  elf_section_list=""$optarg" ${elf_section_list}"
  elf_section_filter_option="-e '$optarg' ${elf_section_filter_option}"
  ;;
  *)
    echo "ERROR: unknown option $opt"
    echo "Try '$0 --help' for more information"
    exit 1
  ;;
  esac
done

## configure basic command
CMD_ELF_NM=${cross_toolchain_prefix}nm
CMD_ELF_ADDR2LINE=${cross_toolchain_prefix}addr2line
CMD_ELF_READELF=${cross_toolchain_prefix}readelf


## Applay option:-h,--help.
if test x"$show_help" = x"yes" ; then
cat << EOF

Usage: $0 [options]
Options: [defaults in brackets after descriptions]

Standard options:
  --help                    Print this message

  --section=NAME            Only display information for section NAME(${elf_section_list})
  --cross-prefix=PREFIX     Cross toolchain prefix($cross_toolchain_prefix)

  --elf=FILE                ELF file path(${elf_file_path})
  --diff-with=FILE          Diff with old statistic report file,and calculate the delta size($diff_with_old_report)

  --prefix-strip=NUM        Strip path prefix(${strip_path_levl})

  --order-reverse           Reverse output order(${output_sort_reverse})

  --skip-line               Skip check seciton for debug line info ($elf_debug_info_check)

Tools Config:
  nm                       (${CMD_ELF_NM})
  addr2line                (${CMD_ELF_ADDR2LINE})
  readelf                  (${CMD_ELF_READELF})
EOF
exit 0
fi
#--fix-line               If can't find debug line info by nm,try addr2line again(${fix_syminfo_by_addr2line})

## check command file
func_check_cmd() {
local cmd_file=`which $1`
if [ ! -x ${cmd_file} ] ; then
echo "Cannot exec command $1"
exit 0
fi
}

## check  file
func_check_file() {
if [ ! -r  "$1" ] ; then
echo "Cannot access file: '$1'"
exit 0
fi
}

func_check_cmd $CMD_ELF_NM
func_check_cmd $CMD_ELF_ADDR2LINE
func_check_file $elf_file_path

## check debug info
if [ "$elf_debug_info_check" = "yes" ];then
func_check_cmd $CMD_ELF_READELF
if [ -z "`$CMD_ELF_READELF ${elf_file_path}  -S | grep \.debug_line`" ]; then
echo "Can't find debug info from $elf_file_path"
exit 0
fi
fi

## 1. dump all symbol info
nm_param_string="-f sysv -l --size-sort --defined-only"

## default order is Z to A, reverse is A to Z
if [ "${output_sort_reverse}" = "no" ];then
nm_param_string="${nm_param_string} -r"
fi

cmd_sequences="${CMD_ELF_NM} ${nm_param_string}  ${elf_file_path} "

## 2. append filter command
if [ -n "$elf_section_list" ] ;then
cmd_sequences="${cmd_sequences} | grep ${elf_section_filter_option}"
else
cmd_sequences="${cmd_sequences} | grep '\.'"
fi
##

## 4 filed info process
## raw format:    Name[1]               Value[2]     Class[3]   Type[4]   Size[5]   Line[6]   Section[7]
## e.g. b8ESRemainderMonitorRunning   | 000d6190 |   B        | OBJECT  | 00000001|         | .bss      /home/xa00107/repo_novatek/re
## striped: 1,2,5,7
create_sym_sort_report()
{
  eval $cmd_sequences | awk -F'|' -v diff_file="${diff_with_old_report}"  -v max_lvl="${strip_path_levl}" '{
    if (NR == 1) {
      output_hdr="SYMBOL,ADDRESS(hex),SIZE(hex),SIZE_OLD(hex),SIZE-SIZE_OLD(dec),SECTION,LINE"
      l_nr = 0
      diff_count = 0
      if ( diff_file != "" ) {
        while ( (getline line < diff_file) > 0 ) {
          l_nr++
          if ( l_nr == 1 ) {
            if ( line != output_hdr ) {
              print "error: wrong header format:" diff_file
              break
            }
          } else {
            f_nr=split(line, line_sec,",")
            diff_count++
            sym_name[diff_count] = line_sec[1]
            sym_size[diff_count] = line_sec[3]
            sym_find[diff_count] = 0
          }
        }
      }
      print output_hdr
    } else if (($1 != "") && ($2 ~/[0-9a-fA-F]+/)) {
      find=0
      old_size=0
      for (i=1; i<=diff_count; i++) {
        if ( (1 != sym_find[i]) && ($1 == sym_name[i])) {
          find = 1
          old_size = sym_size[i]
          sym_find[i] = 1
          break;
        }
      }
      split($7, srcline, " ")
      $7 = srcline[2]
      lvl=0
      while(((max_lvl == -1) || (lvl < max_lvl)) && ($7 ~/^\//)) {
        i=index($7, "/");
        $7=substr($7, i+1);
        lvl++;
        if (lvl < max_lvl) {
          i=index($7, "/");
          $7=substr($7, i);
        }
      }
      print $1 ",0x" $2 ",0x" $5 "," old_size "," (strtonum("0x" $5) - strtonum(old_size))"," srcline[1] "," $7
    }
  }'
}

#eval $cmd_sequences
create_sym_sort_report

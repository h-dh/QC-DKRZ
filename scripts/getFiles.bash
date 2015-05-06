#! /bin/bash

applyRules()
{
   # return 0 :: true

   local retVal
   retVal=1  # go for a file
   local f

   if applySelectionRules $1 ; then
     return 0  # not selected
   fi

   if applyLockRules $1 ; then
     retVal=0  # not selected
   fi

   return $retVal
}

applyLockRules()
{
  # return 0, if locked
  # return 1, if not

  local refName pl sP
  refName=$1

  sP=${subPath}
  test ${#sP} -eq 0 && sP='/.'

  test ${#LOCK_PATH_LIST[*]} -eq 0 -a ${#LOCK_VAR_LIST[*]} -eq 0 \
      && return 1

  # any path selected?
  for(( pl=0 ;  $pl < ${#LOCK_PATH_LIST[*]} ; ++pl )) ; do
    if expr match ${sP} "${LOCK_PATH_LIST[pl]}" &> /dev/null ; then
      if expr match ${refName} "${LOCK_VAR_LIST[pl]}" &> /dev/null ; then
        return 0
      fi
    fi
  done

  return 1  # no locks found
}

applySelectionRules()
{
  # return 1, if selected
  # return 0, if not

  local refName pl sP
  refName=$1

  sP=${subPath}
  test ${#sP} -eq 0 && sP='.'

  test ${#SELECT_PATH_LIST[*]} -eq 0 -a ${#SELECT_VAR_LIST[*]} -eq 0 && return 1

  # any path selected?
  for(( pl=0 ;  $pl < ${#SELECT_PATH_LIST[*]} ; ++pl )) ; do
    if expr match ${sP} "${SELECT_PATH_LIST[pl]}" &> /dev/null ; then
      # special: only the path is to be tested
      test "$1" = PATH && return 1

      # if no variable was selected, then all variables match
      if expr match ${refName} "${SELECT_VAR_LIST[pl]}" &> /dev/null ; then
        return 1
      fi
    fi
  done

  return 0  # no selection for the current sub-path
}

getDateRange()
{

  # echo a date interval, or an instantaneous date none
  local f r

  test $# -eq 0 && return

  f=${1%.nc}

  # e.g. extracts both 186001-186612 and 186001-186612-clim
  r=$( expr match $f '.*_\([[:digit:]]*-[[:digit:]]*.*\)' )

  if [ ${#r} -eq 0 ] ; then
    # no 'appendix allowed'
    r=$( expr match $f '.*_\([[:digit:]]*\)' )
  fi

  test ${#r} -gt 0 && echo $r

  return
}

getFilenameBase()
{
  local f i j k tmp

  unset fBase

  fileNames=($(find ${PROJECT_DATA}/${subPath} -maxdepth 1 -name "*.nc"))
  test ${#fileNames[*]} -eq 0

  fBase=( ${fileNames[*]#${PROJECT_DATA}/${subPath}/} )

  # find all available files: same root, but different time periods
  # is there any qc_*.nc ? Rather circuitous, but safe.
  # works also for names without appended date

  fBase=( ${fBase[*]%.nc} )

  # distinguish asd_range from asdf_range
  for(( i=0 ; i < ${#fBase[*]} ; ++i )) ; do
    tmp=$( getDateRange ${fBase[i]} )
    fBase[i]=${fBase[i]%_${tmp}}
  done

  # remove duplicates
  local sz
  sz=${#fBase[*]}
  for(( i=0 ; i < ${sz} ; ++i )) ; do
    test ${#fBase[i]} -eq 0 && continue

    for(( j=i+1 ; j < ${sz} ; ++j )) ; do
      test "${fBase[i]}" = "${fBase[j]}" && unset fBase[j]
    done
  done

  fBase=( ${fBase[*]} )

  return
}

getNextSubPath()
{
  # subPaths from a temporary file redirected to stdin.
  local ix line

  # pathListFile was connected to stdin in check()
  while : ; do  # for trapping the end

  while read -a line ; do
    # return 1: fade out operation
    if [ ${line[0]} = '---EOF---' ] ; then
      unset fBase
      fBase=''
      subPath=''
      return
    fi

    subPath=${line[1]}
    PROJECT_DATA=${PROJECT_DATAV[${line[0]}]}

    # if read returned true, although there was currently no more entry
    test "${prevLine}" = "${subPath}" && break
    prevLine=${subPath}

    # find the root of all files, stripping of date-periods;
    # puts names to fBase
#trace \
    getFilenameBase

    for(( ix = ${#fBase[*]} - 1 ; ix > -1 ; --ix )) ; do
      if [ ${#fBase[ix]} -gt 0 ] ; then
        # apply rules
        if testLocks ; then
           unset fBase[ix]
           local hasChanged=t
           continue
        fi
      fi
    done

    # remove empty items
    test ${hasChanged:-f} = t && fBase=(${fBase[*]})

    # found unlocked variable (constraint: SHOW_CLEAR=f)
#    mkdir -p ${QA_RESULTS}/data/${subPath}

    return
  done

  done

  return
}

##//! Get paths to all variables scheduled for processing.

##/*!
## The function runs in the back-ground and writes all paths found
## to a temporary file. SELECTion and LOCK assigned in the configuration
## are applied.
##*/

getPaths()
{
  test ${#DEBUG_MANAGER} -eq 0 && pauseX
  # if set -x enabled, then disable for this function

  # Get all paths to sub-dirs that contain netCDF file(s)
  # and list these in a temp file in the directory Project_table.

  # filename
  pathListFile=${SESSION_LOGDIR}/path-list.txt

  PROJECT_DATAV=( ${PROJECT_DATAV[*]//,/ } )

  test ${SHOW_PATH_SEARCH:-f} = t && set -x
  test ${ONLY_SUMMARY} && REUSE_PATH_LIST=t

  # only those paths that are selected and contain netCDF files
  if [ ${REUSE_PATH_LIST:-f} = f \
       -o ! -f ${SESSION_LOGDIR}/path-list.txt ] \
       || ! grep -q -- '---EOF---' ${SESSION_LOGDIR}/path-list.txt ; then
    \rm -f "${pathListFile}"

    if is_TPUT ; then
      echo "getPaths ... " > $TTY
    fi

    getSelectedPaths ${PROJECT_DATAV[*]} &
    getPathPID=$!
  fi

  if [ ${SHOW_PATH_SEARCH:-f} = t ] ; then
    set +x
    wait
    exit
  fi

  # Only pass the loop when the file is already partially filled.
  # Then, it is garantueed that there will be an EOF mark.
  while : ; do
    test ! -e ${pathListFile} && continue

    while [ ! -s ${pathListFile} ] ; do
      sleep 1
    done

    break
  done


  if [ "$( head -n 1 $pathListFile)" = '---EOF---' ] ; then
    lastStatus=1  # exit status of this script
    exit
  fi

  test ${#DEBUG_MANAGER} -eq 0 && pauseX # roll back enabled set -x
}

getSelectedPaths()
{
  # descent recursively into dirs and write all sub-paths,
  # containing at least one netCDF file,
  # into a file.
  test ${SHOW_PATH_SEARCH:-f} = f && set +x

  local currDirs projectDataIndex
  declare -a currDirs projectDataIndex

  local e i j k l s

  if [ ${isStart:-t} = t ] ; then
    isStart=f

    # first depth
    if [ ${#SELECT_PATH_LIST[*]} -eq 0 ] ; then
      SELECT_PATH_LIST=( '.*' )
      SELECT_VAR_LIST=( '.*' )
    fi

    splCount=${#SELECT_PATH_LIST[*]}
    lplCount=${#LOCK_PATH_LIST[*]}

    for(( k=0 ; k < lplCount ; ++k )) ; do
      LOCK_PATH_LIST[${k}]=".*${LOCK_PATH_LIST[k]}"
    done

    local item items sub0 sub1 projectDataPath
    declare -a items projectDataPath

    projectDataPath=( $* ) # actually PROJECT_DATA paths
    recurrCount=0

    set -f
    for(( k=0 ; k < ${#projectDataPath[*]} ; ++k )) do

      for(( l=0 ;  l < ${#SELECT_PATH_LIST[*]} ; ++l )) ; do
        # split selected path into components
        item=${SELECT_PATH_LIST[l]}
        items=( ${item//\// } )

        sub0=

        # look for an alpha-numeric sub-path leading a selected path
        for(( i=0 ; i < ${#items[*]} ; ++i )) ; do
          if expr match ${items[i]} "[[:alnum:]_-]\{${#items[i]}\}" &> /dev/null ; then
            # only accept valid paths
            sub1=${sub0}/"${items[i]}"
            test ! -e ${projectDataPath[k]}$sub1 && break

            sub0=$sub1
          else
            break
          fi
        done

        currDirs[${#currDirs[*]}]=${projectDataPath[k]}$sub0
        basePaths[${#basePaths[*]}]=${projectDataPath[k]}
        projectDataIndex[${#projectDataIndex[*]}]=$k
      done

    done
    set +f

    test ${#currDirs} -eq 0 && currDirs=( ${prjPaths[*]} )
  else
    # deeper recurrence level
    currDirs=( $* )
    recurrCount=$(( recurrCount + 1 ))
  fi

  local entries

  for(( i=0 ; i < ${#currDirs[*]} ; ++i )) ; do
    # Multiple currDirs and basePaths are only possible in the zero-th recursion level.
    # In higher levels, basePath inherits the value from the parent, when getSelected
    # was called there.
    currDir=${currDirs[i]}

    if [ ${recurrCount} -eq 0 ] ; then
      # will be inhereted in deeper recursion levels
      basePath=${basePaths[i]}
      prjDataIndex=${projectDataIndex[i]}
    fi

    entries=( $(ls -d $currDir/* 2> /dev/null) )

    if [ ${HIDDEN_DIRECTORIES:-f} = t ] ; then
      local hidden
      hidden=( $(ls -ad $currDir/.* 2> /dev/null) )
      if [ ${#hidden[*]} -gt 2 ] ; then
        # rm . and ..
        for(( j=${#hidden[*]}-1 ; j > -1 ; --j )) ; do
          test ${hidden[j]##*/} = '.' -o ${hidden[j]##*/} = '..' \
              && unset hidden[${j}]
        done

        entries=( ${entries[*]} ${hidden[*]} )
      fi
    fi

    # check for a variable selection (which is also a directory)
    for(( s=0 ; $s < $splCount ; ++s )) ; do
      if expr match "$currDir" ".*${SELECT_PATH_LIST[s]}" &> /dev/null
      then

        for entry in ${entries[*]} ; do
          # check only netCDF files
          test -d $entry -o ".${entry##*.}" != ".nc" && continue

          e=${entry##*/}

          if expr match "$e" "${SELECT_VAR_LIST[s]}" &> /dev/null
          then

            for(( l=0;  $l < $lplCount ; ++l )) ; do
              if expr match ${currDir} "${LOCK_PATH_LIST[l]}" &> /dev/null
              then
                if expr match ${e} "${LOCK_VAR_LIST[l]}" &> /dev/null
                then
                  # this dir and its descendents are locked. But,
                  # occurrence of multiple variables is possible, thus
                  # no return; just ignore this one and continue.
                  break 2
                fi
              fi
            done

            # found a valid selection; is it unique?
            if ! grep -q ${currDir#${basePath}} $pathListFile \
                  &> /dev/null ; then
              # write sub-path with a leading  slash
              echo "${prjDataIndex} ${currDir#${basePath}/}" >> $pathListFile

              # cap number of file
#              if [ ${NEXT:-0} -gt 0 -a ${NEXT:-0} -eq $(( ++fileCount )) ] ;  then
#                 return
#              fi
            fi

            # this break allows nc-files not only at the end-branches of
            # a directory structure.
            break 2  # only the path is needed
          fi
        done

      fi
    done

    # descend deeper
    for e in ${entries[*]} ; do
      if [ -d $e ] ; then

        # cancel entirely LOCKed directories
        for(( l=0;  $l < $lplCount ; ++l )) ; do
          if expr match ${e} "${LOCK_PATH_LIST[l]}" &> /dev/null
          then
             test "${LOCK_VAR_LIST[l]}" = '.*' && continue 2
          fi
        done

        getSelectedPaths $e
        recurrCount=$(( recurrCount - 1 ))
      fi
    done
  done

  # append End Of File mark
  test ${recurrCount} -eq 0 && echo '---EOF---' >> $pathListFile

  return
}

pauseX()
{
  # toggle between set -x and set +x in a way that
  # restores the original setting after calling twice

  if [ ${isSetX:-t} = t ] ; then
    test "$(set -o |grep xtrace | awk '{print $2}')" = off && return

    # first call
    isSetX=on
  fi

  # restore previous setting
  if [ ${isSetX} = off ] ; then
    set -x
    isSetX=on
  else
    set +x
    isSetX=off
  fi

  return
}

testLocks()
{
  # apply rules, clearings, and test for qc_note files
  if \
#trace \
  applyRules ${fBase[ix]} ; then
    return 0  # not selected
  fi

  return 1
}

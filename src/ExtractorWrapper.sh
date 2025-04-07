#!/bin/bash

# input
PATH_EXE='/home/hagar/Workspace/codes/DXD'
SETUP_FILE=$1
INPUT_FILE_PREFIX=$2
if (( $# != 2 )); then
	echo "Usage: ExtractorWrapper SETUP_FILE INPUT_FILE_PREFIX"
	echo ""
	echo "Action: find all matching 'INPUT_FILE_PREFIX*.DIG' files and extract"
	echo "        each with DxDataExtractor to a single 'INPUT_FILE_PREFIX.DXD'"
	echo " 		  file in the current directory."
	return 1 2> /dev/null || exit 1 # handle either invokation method, source or direct
fi

# process list of files
TIME_STAMP=$(date +%Y%m%d_%H%M%S)
FILE_NAME=$(basename "${INPUT_FILE_PREFIX}")
OUT_FILE="${FILE_NAME}.DXD"
LOG_FILE="${OUT_FILE}_${TIME_STAMP}.log"
echo "Starting at ${TIME_STAMP}:" >> ${LOG_FILE}
for DIG_FILE in $(find ${INPUT_FILE_PREFIX}*.dat)
do
    echo "Extracting ${DIG_FILE} (pid ${!}) ---> output: ${OUT_FILE}, log: ${LOG_FILE}"
#	./DxDataExtractor_from_gera "${SETUP_FILE}" "${DIG_FILE}" "${OUT_FILE}" >> "${LOG_FILE}" 2>&1 </dev/null
#	echo	"$PATH_EXE/DxDataExtractor "./" "${SETUP_FILE}" "${DIG_FILE}" "${OUT_FILE}"   "
	$PATH_EXE/DxDataExtractor "./"  "${SETUP_FILE}" "${DIG_FILE}" "${OUT_FILE}"

done

# EOF
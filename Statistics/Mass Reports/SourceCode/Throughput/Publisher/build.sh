rm MassReport.h
rm MassReport.cxx
rm MassReportPubSubTypes.h
rm MassReportPubSubTypes.cxx

fastrtpsgen *.idl

cmake .

make
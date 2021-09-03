rm BasicTransport.h
rm BasicTransport.cxx
rm BasicTransportPubSubTypes.h
rm BasicTransportPubSubTypes.cxx

fastrtpsgen *.idl

cmake .

make
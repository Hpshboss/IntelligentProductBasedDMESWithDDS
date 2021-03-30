# Parsing Idl File For Picture Sample

### Steps

1. Only upload sample.idl to device
2. Enter the following command ```fastrtpsgen sample.idl```

***

### Results

This must have generated the following files:

- sample.cxx: HelloWorld type definition.

- sample.h: Header file for HelloWorld.cxx.

- samplePubSubTypes.cxx: Serialization and Deserialization code for the HelloWorld type.

- samplePubSubTypes.h: Header file for HelloWorldPubSubTypes.cxx.

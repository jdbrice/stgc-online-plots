Online plots for the Forward sTGC Tracker
===

# How-to

```sh
ln -sf /path/to/daq/file/raw.daq input.daq #set input file
starver dev 
cons
./testFtt.sh
```

will output a PDF file `fttBuilder.pdf`


# Development
The FttBuilder code is in:
- `OnlTools/Jevp/StJevpBuilders/fttBuilder.h`
- `OnlTools/Jevp/StJevpBuilders/fttBuilder.cxx`

Update these, rebuild
```sh
cons
```



var pkgargs = [ 

{ 
  name : "ti.mas.swtools",
  arg  : [ "all" ] 
},

{ 
  name : "ti.mas.types",
  arg  : [ "all" ] 
},

{ 
  name : "ti.mas.util",
  arg  : [ "all" ] 
},

{ 
  name : "ti.mas.fract",
  arg  : [ "all" ] 
},

{ 
  name : "ti.mas.asm",
  arg  : [ "all" ] 
},

{ 
  name : "ti.mas.aer",
  arg  : [ "c64Ple" ] 
},

{ 
  name : "ti.mas.eco",
  arg  : [ "c64Ple", "gcarmv6", "debug" ] 
},

{ 
  name : "ti.mas.apu",
  arg  : [ "c64Ple", "debug" ] 
},

{ 
  name : "ti.mas.devnode",
  arg  : [ "c64Ple", "gcarmv6", "debug" ]
},

{ 
  name : "ti.mas.afe",
  arg  : [ "c64Ple", "debug" ] 
},

{ 
  name : "ti.mas.examples.devnodeServer",
  arg  : [ "c64Ple", "dm3730", "debug" ] 
},

{ 
  name : "lab126.full_server",
  arg  : [ "c64Ple", "dm3730", "debug" ] 
},

{ 
  name : "ti.mas.examples.devnodeApp",
  arg  : [ "gcarmv6", "debug" ] 
},


// Add pkg params before this line

//{ // This one is a toy example
//  name : "ti.mas.examples.vpapp",
//  arg  : [ "Arg0" , "Arg1", "Arg2", "debug" ]
//},

{ // Don't remove this
  name : "",
  arg  : [ ] 
}

];


function getPkgArgs(pkgname, arguments)
{
    var i = 0;

    arguments.length = 0;

    while (pkgargs[i].name != "") 
    {
        if(pkgargs[i].name == pkgname)
        {
            for(var j=0; j < pkgargs[i].arg.length; j++)
            {
                arguments[j] = pkgargs[i].arg[j];
            }
 
            arguments.length = pkgargs[i].arg.length;
            break;
        }
        else
            ++i;
    }
}


function showPkgArgs(pkgname, arguments)
{
    print("Pkg " + pkgname + " got " + arguments.length + " arguments:");
    for (var k = 0; k < arguments.length; k++) {
        print(arguments[k]);
    }
}







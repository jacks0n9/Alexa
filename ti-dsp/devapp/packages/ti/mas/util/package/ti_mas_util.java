/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-u17
 */
import java.util.*;
import org.mozilla.javascript.*;
import xdc.services.intern.xsr.*;
import xdc.services.spec.*;

public class ti_mas_util
{
    static final String VERS = "@(#) xdc-u17\n";

    static final Proto.Elm $$T_Bool = Proto.Elm.newBool();
    static final Proto.Elm $$T_Num = Proto.Elm.newNum();
    static final Proto.Elm $$T_Str = Proto.Elm.newStr();
    static final Proto.Elm $$T_Obj = Proto.Elm.newObj();

    static final Proto.Fxn $$T_Met = new Proto.Fxn(null, null, 0, -1, false);
    static final Proto.Map $$T_Map = new Proto.Map($$T_Obj);
    static final Proto.Arr $$T_Vec = new Proto.Arr($$T_Obj);

    static final XScriptO $$DEFAULT = Value.DEFAULT;
    static final Object $$UNDEF = Undefined.instance;

    static final Proto.Obj $$Package = (Proto.Obj)Global.get("$$Package");
    static final Proto.Obj $$Module = (Proto.Obj)Global.get("$$Module");
    static final Proto.Obj $$Instance = (Proto.Obj)Global.get("$$Instance");
    static final Proto.Obj $$Params = (Proto.Obj)Global.get("$$Params");

    static final Object $$objFldGet = Global.get("$$objFldGet");
    static final Object $$objFldSet = Global.get("$$objFldSet");
    static final Object $$proxyGet = Global.get("$$proxyGet");
    static final Object $$proxySet = Global.get("$$proxySet");
    static final Object $$delegGet = Global.get("$$delegGet");
    static final Object $$delegSet = Global.get("$$delegSet");

    Scriptable xdcO;
    Session ses;
    Value.Obj om;

    boolean isROV;
    boolean isCFG;

    Proto.Obj pkgP;
    Value.Obj pkgV;

    ArrayList<Object> imports = new ArrayList<Object>();
    ArrayList<Object> loggables = new ArrayList<Object>();
    ArrayList<Object> mcfgs = new ArrayList<Object>();
    ArrayList<Object> proxies = new ArrayList<Object>();
    ArrayList<Object> sizes = new ArrayList<Object>();
    ArrayList<Object> tdefs = new ArrayList<Object>();

    void $$IMPORTS()
    {
        Global.callFxn("loadPackage", xdcO, "ti.mas.types");
        Global.callFxn("loadPackage", xdcO, "ti.mas.fract");
        Global.callFxn("loadPackage", xdcO, "ti.mas.asm");
        Global.callFxn("loadPackage", xdcO, "ti.xdais");
        Global.callFxn("loadPackage", xdcO, "xdc");
        Global.callFxn("loadPackage", xdcO, "xdc.corevers");
        Global.callFxn("loadPackage", xdcO, "ti.mas.swtools");
    }

    void $$OBJECTS()
    {
        pkgP = (Proto.Obj)om.bind("ti.mas.util.Package", new Proto.Obj());
        pkgV = (Value.Obj)om.bind("ti.mas.util", new Value.Obj("ti.mas.util", pkgP));
    }

    void Version$$OBJECTS()
    {
        Proto.Obj po, spo;
        Value.Obj vo;

        po = (Proto.Obj)om.bind("ti.mas.util.Version.Module", new Proto.Obj());
        vo = (Value.Obj)om.bind("ti.mas.util.Version", new Value.Obj("ti.mas.util.Version", po));
        pkgV.bind("Version", vo);
        // decls 
    }

    void Version$$CONSTS()
    {
        // module Version
        om.bind("ti.mas.util.Version.MAJOR", 4L);
        om.bind("ti.mas.util.Version.MINOR", 7L);
        om.bind("ti.mas.util.Version.PATCH", 2L);
        om.bind("ti.mas.util.Version.BUILD", 0L);
        om.bind("ti.mas.util.Version.QUALITY", 3L);
        om.bind("ti.mas.util.Version.BTYPE", 0L);
        om.bind("ti.mas.util.Version.WIDTH", 0L);
        om.bind("ti.mas.util.Version.PKGSTD", 2L);
    }

    void Version$$CREATES()
    {
        Proto.Fxn fxn;
        StringBuilder sb;

    }

    void Version$$FUNCTIONS()
    {
        Proto.Fxn fxn;

    }

    void Version$$SIZES()
    {
    }

    void Version$$TYPES()
    {
        Scriptable cap;
        Proto.Obj po;
        Proto.Str ps;
        Proto.Typedef pt;
        Object fxn;

        po = (Proto.Obj)om.findStrict("ti.mas.util.Version.Module", "ti.mas.util");
        po.init("ti.mas.util.Version.Module", om.findStrict("ti.mas.swtools.IVersion2.Module", "ti.mas.util"));
                po.addFld("$hostonly", $$T_Num, 1, "r");
                po.addFld("QUALITY_PREALPHA", Proto.Elm.newCNum("(xdc_UInt16)"), 0L, "rh");
                po.addFld("QUALITY_ALPHA", Proto.Elm.newCNum("(xdc_UInt16)"), 1L, "rh");
                po.addFld("QUALITY_BETA", Proto.Elm.newCNum("(xdc_UInt16)"), 2L, "rh");
                po.addFld("QUALITY_GA", Proto.Elm.newCNum("(xdc_UInt16)"), 3L, "rh");
                po.addFld("BTYPE_STANDARD", Proto.Elm.newCNum("(xdc_UInt16)"), 0L, "rh");
                po.addFld("BTYPE_ENGINEERING", Proto.Elm.newCNum("(xdc_UInt16)"), 1L, "rh");
                po.addFld("BTYPE_DEBUG", Proto.Elm.newCNum("(xdc_UInt16)"), 2L, "rh");
                po.addFld("MAJOR", Proto.Elm.newCNum("(xdc_UInt16)"), 4L, "rh");
                po.addFld("MINOR", Proto.Elm.newCNum("(xdc_UInt16)"), 7L, "rh");
                po.addFld("PATCH", Proto.Elm.newCNum("(xdc_UInt16)"), 2L, "rh");
                po.addFld("BUILD", Proto.Elm.newCNum("(xdc_UInt16)"), 0L, "rh");
                po.addFld("QUALITY", Proto.Elm.newCNum("(xdc_UInt16)"), 3L, "rh");
                po.addFld("BTYPE", Proto.Elm.newCNum("(xdc_UInt16)"), 0L, "rh");
                po.addFld("WIDTH", Proto.Elm.newCNum("(xdc_UInt16)"), 0L, "rh");
                po.addFld("PKGSTD", Proto.Elm.newCNum("(xdc_UInt16)"), 2L, "rh");
                po.addFxn("getString", (Proto.Fxn)om.findStrict("ti.mas.swtools.IVersion2$$getString", "ti.mas.util"), $$UNDEF);
    }

    void Version$$ROV()
    {
    }

    void $$SINGLETONS()
    {
        pkgP.init("ti.mas.util.Package", (Proto.Obj)om.findStrict("xdc.IPackage.Module", "ti.mas.util"));
        Scriptable cap = (Scriptable)Global.callFxn("loadCapsule", xdcO, "ti/mas/util/package.xs");
        om.bind("xdc.IPackage$$capsule", cap);
        Object fxn;
                fxn = Global.get(cap, "init");
                if (fxn != null) pkgP.addFxn("init", (Proto.Fxn)om.findStrict("xdc.IPackage$$init", "ti.mas.util"), fxn);
                fxn = Global.get(cap, "close");
                if (fxn != null) pkgP.addFxn("close", (Proto.Fxn)om.findStrict("xdc.IPackage$$close", "ti.mas.util"), fxn);
                fxn = Global.get(cap, "validate");
                if (fxn != null) pkgP.addFxn("validate", (Proto.Fxn)om.findStrict("xdc.IPackage$$validate", "ti.mas.util"), fxn);
                fxn = Global.get(cap, "exit");
                if (fxn != null) pkgP.addFxn("exit", (Proto.Fxn)om.findStrict("xdc.IPackage$$exit", "ti.mas.util"), fxn);
                fxn = Global.get(cap, "getLibs");
                if (fxn != null) pkgP.addFxn("getLibs", (Proto.Fxn)om.findStrict("xdc.IPackage$$getLibs", "ti.mas.util"), fxn);
                fxn = Global.get(cap, "getSects");
                if (fxn != null) pkgP.addFxn("getSects", (Proto.Fxn)om.findStrict("xdc.IPackage$$getSects", "ti.mas.util"), fxn);
        pkgP.bind("$capsule", cap);
        pkgV.init2(pkgP, "ti.mas.util", Value.DEFAULT, false);
        pkgV.bind("$name", "ti.mas.util");
        pkgV.bind("$category", "Package");
        pkgV.bind("$$qn", "ti.mas.util.");
        pkgV.bind("$vers", Global.newArray("4, 0, 8, 0"));
        Value.Map atmap = (Value.Map)pkgV.getv("$attr");
        atmap.seal("length");
        imports.clear();
        imports.add(Global.newArray("ti.mas.types", Global.newArray(5, 0, 7, 0)));
        imports.add(Global.newArray("ti.mas.fract", Global.newArray(2, 0, 7, 0)));
        imports.add(Global.newArray("ti.mas.asm", Global.newArray(3, 0, 3, 0)));
        imports.add(Global.newArray("ti.xdais", Global.newArray(1, 2,0, 0)));
        imports.add(Global.newArray("*ti.mas.swtools", Global.newArray(2, 0, 4, 0)));
        pkgV.bind("$imports", imports);
        StringBuilder sb = new StringBuilder();
        sb.append("var pkg = xdc.om['ti.mas.util'];\n");
        sb.append("if (pkg.$vers.length >= 3) {\n");
            sb.append("pkg.$vers.push(Packages.xdc.services.global.Vers.getDate(xdc.csd() + '/..'));\n");
        sb.append("}\n");
        sb.append("pkg.build.libraries = [\n");
        sb.append("];\n");
        sb.append("pkg.build.libDesc = [\n");
        sb.append("];\n");
        sb.append("if('suffix' in xdc.om['xdc.IPackage$$LibDesc']) {\n");
        sb.append("}\n");
        Global.eval(sb.toString());
    }

    void Version$$SINGLETONS()
    {
        Proto.Obj po;
        Value.Obj vo;

        vo = (Value.Obj)om.findStrict("ti.mas.util.Version", "ti.mas.util");
        po = (Proto.Obj)om.findStrict("ti.mas.util.Version.Module", "ti.mas.util");
        vo.init2(po, "ti.mas.util.Version", $$DEFAULT, false);
        vo.bind("Module", po);
        vo.bind("$category", "Module");
        vo.bind("$capsule", $$UNDEF);
        vo.bind("$package", om.findStrict("ti.mas.util", "ti.mas.util"));
        tdefs.clear();
        proxies.clear();
        mcfgs.clear();
        vo.bind("$$tdefs", Global.newArray(tdefs.toArray()));
        vo.bind("$$proxies", Global.newArray(proxies.toArray()));
        vo.bind("$$mcfgs", Global.newArray(mcfgs.toArray()));
        ((Value.Arr)pkgV.getv("$modules")).add(vo);
        ((Value.Arr)om.findStrict("$modules", "ti.mas.util")).add(vo);
        vo.bind("$$instflag", 0);
        vo.bind("$$iobjflag", 1);
        vo.bind("$$sizeflag", 1);
        vo.bind("$$dlgflag", 0);
        vo.bind("$$iflag", 1);
        vo.bind("$$romcfgs", "|");
        Proto.Str ps = (Proto.Str)vo.find("Module_State");
        if (ps != null) vo.bind("$object", ps.newInstance());
        vo.bind("$$meta_iobj", om.has("ti.mas.util.Version$$instance$static$init", null) ? 1 : 0);
        vo.bind("$$fxntab", Global.newArray());
        vo.bind("$$logEvtCfgs", Global.newArray());
        vo.bind("$$errorDescCfgs", Global.newArray());
        vo.bind("$$assertDescCfgs", Global.newArray());
        Value.Map atmap = (Value.Map)vo.getv("$attr");
        atmap.seal("length");
        pkgV.bind("Version", vo);
        ((Value.Arr)pkgV.getv("$unitNames")).add("Version");
    }

    void $$INITIALIZATION()
    {
        Value.Obj vo;

        if (isCFG) {
        }//isCFG
        Global.callFxn("module$meta$init", (Scriptable)om.findStrict("ti.mas.util.Version", "ti.mas.util"));
        Global.callFxn("init", pkgV);
        ((Value.Obj)om.getv("ti.mas.util.Version")).bless();
        ((Value.Arr)om.findStrict("$packages", "ti.mas.util")).add(pkgV);
    }

    public void exec( Scriptable xdcO, Session ses )
    {
        this.xdcO = xdcO;
        this.ses = ses;
        om = (Value.Obj)xdcO.get("om", null);

        Object o = om.geto("$name");
        String s = o instanceof String ? (String)o : null;
        isCFG = s != null && s.equals("cfg");
        isROV = s != null && s.equals("rov");

        $$IMPORTS();
        $$OBJECTS();
        Version$$OBJECTS();
        Version$$CONSTS();
        Version$$CREATES();
        Version$$FUNCTIONS();
        Version$$SIZES();
        Version$$TYPES();
        if (isROV) {
            Version$$ROV();
        }//isROV
        $$SINGLETONS();
        Version$$SINGLETONS();
        $$INITIALIZATION();
    }
}

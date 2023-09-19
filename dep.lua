local pkg = premake.extensions.pkg

libdirs({ pkg:scriptDir() .. string.format("/Bin/%s-%s-", os.host(), pkg.arch) .. "%{cfg.buildcfg}" })
links({ "CommonBuild" })
externalincludedirs({ pkg:scriptDir() .. "/Inc/" })

pkgdeps({ "mimalloc" })

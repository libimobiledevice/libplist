%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}

Name:          libplist
Version:       1.13.build
Release:       0%{?dist}
Summary:       Library for manipulating Apple Binary and XML Property Lists

Group:         System Environment/Libraries
License:       LGPLv2+
URL:           http://www.libimobiledevice.org/
Source0:       http://www.libimobiledevice.org/downloads/%{name}-%{version}.tar.gz

BuildRequires: swig
BuildRequires: gcc-c++

%description
libplist is a library for manipulating Apple Binary and XML Property Lists

%package devel
Summary: Development package for libplist
Group: Development/Libraries
Requires: libplist = %{version}-%{release}
Requires: pkgconfig

%description devel
%{name}, development headers and libraries.

%prep
%setup -q

%build
./autogen.sh --prefix=/usr --without-cython --libdir=/usr/lib64 --enable-static=no --enable-shared=yes
make

%install
make DESTDIR=$RPM_BUILD_ROOT install
rm $RPM_BUILD_ROOT/usr/lib64/*.la

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING.LESSER README
%{_bindir}/plistutil
%{_libdir}/libplist.so.*
%{_libdir}/libplist++.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/libplist.pc
%{_libdir}/pkgconfig/libplist++.pc
%{_libdir}/libplist.so
%{_libdir}/libplist++.so
%{_includedir}/plist

%changelog
* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 1.10-4
- Mass rebuild 2014-01-24

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 1.10-3
- Mass rebuild 2013-12-27

* Tue Oct  8 2013 Matthias Clasen <mclasen@redhat.com> - 1.10-2
- Disable strict aliasing (related: #884099)

* Tue Mar 19 2013 Peter Robinson <pbrobinson@fedoraproject.org> 1.10-1
- New upstream 1.10 release

* Mon Mar 18 2013 Peter Robinson <pbrobinson@fedoraproject.org> 1.9-1
- New upstream 1.9 release

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.8-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.8-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Wed Apr 11 2012 Peter Robinson <pbrobinson@fedoraproject.org> - 1.8-4
- Fix python bindings

* Wed Apr 11 2012 Rex Dieter <rdieter@fedoraproject.org> 1.8-3
- fix ftbfs, work harder to ensure CMAKE_INSTALL_LIBDIR macro is correct 

* Fri Mar 23 2012 Peter Robinson <pbrobinson@fedoraproject.org> - 1.8-2
- Fix RPATH issue with cmake, disable parallel build as it causes other problems

* Thu Jan 12 2012 Peter Robinson <pbrobinson@fedoraproject.org> - 1.8-1
- 1.8 release

* Mon Sep 26 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 1.7-1
- 1.7 release

* Sat Jun 25 2011 Peter Robinson <pbrobinson@fedoraproject.org> 1.6-1
- 1.6 release

* Mon Jun 13 2011 Peter Robinson <pbrobinson@fedoraproject.org> 1.5-1
- 1.5 release

* Tue Mar 22 2011 Peter Robinson <pbrobinson@fedoraproject.org> 1.4-1
- stable 1.4 release

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Wed Jul 21 2010 David Malcolm <dmalcolm@redhat.com> - 1.3-2
- Rebuilt for https://fedoraproject.org/wiki/Features/Python_2.7/MassRebuild

* Tue Apr 20 2010 Peter Robinson <pbrobinson@fedoraproject.org> 1.3-1
- Upstream stable 1.3 release

* Sat Jan 23 2010 Peter Robinson <pbrobinson@fedoraproject.org> 1.2-1
- Upstream stable 1.2 release

* Sat Jan  9 2010 Peter Robinson <pbrobinson@fedoraproject.org> 1.0.0-5
- Updated to the new python sysarch spec file reqs

* Mon Dec  7 2009 Peter Robinson <pbrobinson@fedoraproject.org> 1.0.0-4
- and once more with feeling

* Mon Dec  7 2009 Peter Robinson <pbrobinson@fedoraproject.org> 1.0.0-3
- Further updated fixes for the spec file

* Mon Dec  7 2009 Peter Robinson <pbrobinson@fedoraproject.org> 1.0.0-2
- Drop upstreamed patch

* Mon Dec  7 2009 Peter Robinson <pbrobinson@fedoraproject.org> 1.0.0-1
- Upstream stable 1.0.0 release

* Thu Oct 29 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.16-3
- Actually add patch for python

* Thu Oct 29 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.16-2
- Add python patch and c++ bindings

* Thu Oct 29 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.16-1
- New upstream 0.16 release

* Tue Oct 20 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.15-1
- New upstream 0.15 release

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.13-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Mon May 11 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.13-1
- New upstream 0.13 release

* Mon May 11 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.12-2
- Further review updates

* Sun May 10 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.12-1
- Update to official tarball release, some review fixes

* Sun May 10 2009 Peter Robinson <pbrobinson@fedoraproject.org> 0.12.0-0.1
- Initial package


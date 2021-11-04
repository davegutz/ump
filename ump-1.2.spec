Summary: A dual-camera vision system program that calls balls and strikes.
Name: ump
Version: 1.2
Release: 1
License: Commercial
Group: Applications/Miscellaneous
URL: http://mywebpages.comcast.net/ump/index.html
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
Two framegrabbers are threaded to collect simultaneous motion data, group the data into balls, and measure ball motion to determine if it goes through the strike zone.  It's a digital umpire.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}
make check -s


%install
rm -rf $RPM_BUILD_ROOT
%makeinstall
#%find_lang %{name}

rm -f $RPM_BUILD_ROOT%{_infodir}/dir
find $RPM_BUILD_ROOT -type f -name "*.la" -exec rm -f {} ';'

%clean
rm -rf $RPM_BUILD_ROOT

#%files -f %{name}.lang
%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog NEWS README TODO INSTALL exmask.ps THANKS
%{_bindir}/*
%{_infodir}/*
%{_datadir}/*
#%{_libdir}/*.so.*
#%{_datadir}/%{name}
#%{_mandir}/man[^3]/*

%changelog
* Sun Jan 24 2010 Dave Gutz <davegutz@alum.mit.edu> 1.2-1
- mac port fix
* Sun Jan 10 2010 Dave Gutz <davegutz@alum.mit.edu> 1.2-0
- mac port
* Tue Dec 18 2007 Dave Gutz <davegutz@alum.mit.edu> 1.1-1
- RHEL5 clean compile (except STRTAB initialization)
* Sun Dec 16 2007 Dave Gutz <davegutz@alum.mit.edu> 1.1-0
- RHEL5
* Fri Feb 16 2007 Dave Gutz <davegutz@alum.mit.edu> 1.0-6
- style
* Sun Feb 11 2007 Dave Gutz <davegutz@alum.mit.edu> 1.0-5
- style
* Fri Jan 26 2007 Dave Gutz <davegutz@alum.mit.edu> 1.0-4
- Leaks in Blink
* Mon Jan 22 2007 Dave Gutz <davegutz@alum.mit.edu> 1.0-3
- Some checkCal cleanup
* Mon Jan 15 2007 Dave Gutz <davegutz@alum.mit.edu> 1.0-2
- Some leak fixes
* Sat Dec 22 2006 Dave Gutz <davegutz@alum.mit.edu> 1.0-1
- lyx revision
* Fri Sep 15 2006 Dave Gutz <davegutz@alum.mit.edu> 1.0-0
- done?
* Wed Aug 23 2006 Dave Gutz <davegutz@alum.mit.edu> 0.15-4
- debug
* Sun Aug  6 2006 Dave Gutz <davegutz@alum.mit.edu> 0.15-3
- SY only
* Wed Jul 12 2006 Dave Gutz <davegutz@alum.mit.edu> 0.15-2
- zap and LeakTracer
* Mon Jul 10 2006 Dave Gutz <davegutz@alum.mit.edu> 0.15-1
- SY and centerpoint corrections to fix accuracy problems. (2 in --> 0.25 in)
* Sat May 27 2006 Dave Gutz <davegutz@alum.mit.edu> 0.14-2
- Alpha release
* Sat May 20 2006 Dave Gutz <davegutz@alum.mit.edu> 0.14-1
- Alpha release
* Sun May  7 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-16
- Progress on Blink
* Mon May  1 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-15
- Progress on Blink
* Sat Apr 29 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-14
- Progress on Blink
* Mon Apr 24 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-13
- Progress on Blink
* Tue Apr 18 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-12
- Progress on Blink
* Wed Mar  8 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-11
- Progress on Blink
* Mon Feb 20 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-10
- Progress on Blink
* Sun Feb 12 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-9
- port to AMD
* Sat Feb  4 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-8
- debug glade
* Sat Jan 14 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-7
- debug glade
* Mon Jan  9 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-6
- debug glade
* Sun Jan  8 2006 Dave Gutz <davegutz@alum.mit.edu> 0.13-5
- glade
* Sat Dec 24 2005 Dave Gutz <davegutz@alum.mit.edu> 0.13-4
- tim2clk
* Sat Dec 24 2005 Dave Gutz <davegutz@alum.mit.edu> 0.13-3
- Cleanup TODO
* Thu Dec 22 2005 Dave Gutz <davegutz@alum.mit.edu> 0.13-2
- Mask.h Mask.cc
* Thu Dec 22 2005 Dave Gutz <davegutz@alum.mit.edu> 0.13-1
- draw mask from mms
* Sat Dec 18 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-7
- crunch ppms, debug triangulation, n & m from .cal
* Sat Dec 03 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-6
- isnan and negative dTball
* Thu Dec 01 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-5
- Register error
* Sat Nov 25 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-4
- Basement
* Sat Oct 30 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-3
- Time reject fixes
* Sat Oct 22 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-2
- Debug in driveway
* Mon Sep 19 2005 Dave Gutz <davegutz@alum.mit.edu> 0.12-1
- Initial RHEL4 build.
* Sat Sep 10 2005 Dave Gutz <davegutz@alum.mit.edu> 0.11-1
- Initial build.
* Wed Feb 23 2005 Dave Gutz <davegutz@alum.mit.edu> 0.10-1
- Initial build.



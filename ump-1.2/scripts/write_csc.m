function write_csc(fn, val, Nrhs, rhs, guess, exact, Title, Key,...
		   Type, Ptrfmt, Indfmt, Valfmt, Rhsfmt, Rhstype)
% write_csc:  Output a matrix in Harwell-Boeing format (CSC)
% Inputs:
% fn      Filename string
% val     Matrix, MxN
% Nrhs    Number of right-hand-sides stored with matrix
% rhs     Right-hand-side vector
% guess   Guess vector (all zeros) Type = R:  M*Nrhs
%                                  Type = C:  2*M*Nrhs
%                                  Type = P:  ?
% exact   Exact solution vector
% Title   1-72 char long title string
% Key     1-8  char long key string
% Type    3-character matrix type
% Ptrfmt  '13I6
% Indfmt  '16I5
% Valfmt  '3E26.18'
% Rhsfmt  '3E26.18'
% Rhstype col 1: F for full storage or M for same format as matrix
%         col 2: G if a guess vector supplied
%         col 3: X if an exact solution vector supplied
%

% Local:
% M       Number rows stored matrix
% N       Number columns stored matrix
% nz      Number nonzeros in stored matrix
% colptr  Column pointers, N
% rowind  Row indeces, nz
% val     Matrix


%  The writeHB_mat_double function opens the named file and writes the specified
%  matrix and optional auxillary vector(s) to that file in Harwell-Boeing
%  format.  The format arguments (Ptrfmt,Indfmt,Valfmt, and Rhsfmt) are
%  character strings specifying "Fortran-style" output formats -- as they
%  would appear in a Harwell-Boeing file.  They are used to produce output
%  which is as close as possible to what would be produced by Fortran code,
%  but note that "D" and "P" edit descriptors are not supported.
%  If NULL, the following defaults will be used:
%                    Ptrfmt = Indfmt = "(8I10)"
%                    Valfmt = Rhsfmt = "(4E20.13)"
%
%  For a description of the Harwell Boeing standard, see:                  
%            Duff, et al.,  ACM TOMS Vol.15, No.1, March 1989             
%
%++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
%                       Harwell-Boeing File I/O in C
%                                V. 1.0
%
%           National Institute of Standards and Technology, MD.
%                             K.A. Remington
%
%++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


%  int writeHB_mat_double(const char* filename, int M, int N, 
%                        int nz, const int colptr[], const int rowind[], 
%                        const double val[], int Nrhs, const double rhs[], 
%                        const double guess[], const double exact[],
%                        const char* Title, const char* Key, const char* Type, 
%                        char* Ptrfmt, char* Indfmt, char* Valfmt, char* Rhsfmt,
%                        const char* Rhstype)

   [N, M] = size(val);
   AS=sparse(val);
   [rowind,I,VS]=find(AS);
   nz = length(VS);
   [NS,MS]=size(AS);
   num=0;
   values = zeros(nz,1);
   colptr=zeros(M+1,1);
   for j=1:M,
      if any(val(:,j)),
         [rowlocs] = find(sparse(val(:,j)));
      end
      for i=1:N,
         if val(i,j) ~=0,
            num=num+1;
            values(num)=val(i,j);
            if i == rowlocs(1),
               colptr(j) = num;
            end
         end
      end
   end
   end
   colptr(M+1) = num+1;



    out_file = fopen(fn, 'w');

    i=0;j=0;entry=0;offset=0;acount=0;linemod=0;
    totcrd=0;ptrcrd=0;indcrd=0;valcrd=0;rhscrd=0;
    nvalentries=0;nrhsentries=0;
    Ptrperline=0;Ptrwidth=0;Indperline=0;Indwidth=0;
    Rhsperline=0;Rhswidth=0;Rhsprec=0;
    Rhsflag=0;
    Valperline=0;Valwidth=0;Valprec=0;
    Valflag=0;           % Indicates 'E','D', or 'F' float format

    pformat=[];iformat=[];vformat=[];rformat=[];

    if Type(1) == 'C',
         nvalentries = 2*nz;
         nrhsentries = 2*M;
    else
         nvalentries = nz;
         nrhsentries = M;
    end

    if Ptrfmt == [], Ptrfmt = ['(8I10)          ']; end
    Ptrperline=8;Ptrwidth=10;  % Arbitrary until have parse function for Ptrfmt.
    pformat=sprintf('%% %di', Ptrwidth);
    ptrcrd = floor((N+1)/Ptrperline);
    if rem((N+1),Ptrperline) ~= 0, ptrcrd=ptrcrd+1; end

    if Indfmt == [], Indfmt=Ptrfmt; end
    Indperline=8;Indwidth=10;  % Arbitrary until have parse function for Indfmt.
    iformat=pformat;
    indcrd = floor(nz/Indperline);
    if rem(nz,Indperline) ~= 0, indcrd=indcrd+1; end


    if Type(1) ~= 'P',     % Skip if pattern only
      if Valfmt == [],
         Valfmt = ['(4E20.13)       '];
         Valperline=4;Valwidth=20;Valprec=13;Valflag='E';  % Need parser.
      end
      %if (Valflag == 'D') *strchr(Valfmt,'D') = 'E';
      %if (Valflag == 'F')
      %    sprintf(vformat,"%% %d.%df",Valwidth,Valprec);
      % else
      %     sprintf(vformat,"%% %d.%dE",Valwidth,Valprec);
      vformat=sprintf('%% %d.%dE', Valwidth, Valprec);
      valcrd = floor(nvalentries/Valperline);
      if rem(nvalentries,Valperline) ~= 0,
         valcrd=valcrd+1;
      end
    else
       valcrd = 0;
    end %Type(1)~=P



    if Nrhs > 0,
       if Rhsfmt == [], Rhsfmt = Valfmt; end
       Rhsperline=4;Rhswidth=20;Rhsprec=13;Rhsflag='E';  % Need parser.
       rformat=sprintf('%% %d.%dE', Rhswidth, Rhsprec);
       rhscrd = floor(nrhsentries/Rhsperline);
       if rem(nrhsentries,Rhsperline) ~= 0, rhscrd=rhscrd+1; end
       if Rhstype(2) == 'G', rhscrd=rhscrd+1; end       
       if Rhstype(3) == 'X', rhscrd=rhscrd+1; end       
       rhscrd=rhscrd*Nrhs;
    else
       rhscrd = 0;
    end %Nrhs>0
    totcrd = 4+ptrcrd+indcrd+valcrd+rhscrd;


%  Print header information:

    fprintf(out_file,'%-72s%-8s\n%14d%14d%14d%14d%14d\n',Title, Key, totcrd,...
            ptrcrd, indcrd, valcrd, rhscrd);
    fprintf(out_file,'%3s%11s%14d%14d%14d\n',Type,'          ', M, N, nz);
    fprintf(out_file,'%-16s%-16s%-20s', Ptrfmt, Indfmt, Valfmt);
    if Nrhs != 0,
       % Print Rhsfmt on fourth line and                                   
       % optional fifth header line for auxillary vector information:
       fprintf(out_file,'%-20s\n%-14s%d\n',Rhsfmt,Rhstype,Nrhs);
    else 
       fprintf(out_file,'\n');
    end % Nrhs!=0

    SP_base=1;
    offset = 1-SP_base;  % if base 0 storage is declared (via macro definition),
                          % then storage entries are offset by 1                

%  Print column pointers: 
   for i=1:M+1,
      entry = colptr(i)+offset;
      fprintf(out_file,pformat,entry);
      if rem( i, Ptrperline) == 0, fprintf(out_file,'\n');end
   end

   if rem( N+2, Ptrperline) ~= 0, fprintf(out_file,'\n');end

%  Print row indices:  
   for i=1:nz,
      entry = rowind(i)+offset;
      fprintf(out_file,iformat,entry);
      if rem(i,Indperline) == 0, fprintf(out_file,'\n');end
   end

   if rem(nz, Indperline) ~= 0, fprintf(out_file,'\n');end


%  Print values:       
   if Type(1) ~= 'P',          % Skip if pattern only
    for i=1:nvalentries,
       fprintf(out_file,vformat,values(i));
       if rem(i,Valperline) == 0, fprintf(out_file,'\n'); end
    end

    if rem(nvalentries,Valperline) ~= 0, fprintf(out_file,'\n'); end

%   If available,  print right hand sides, 
%          guess vectors and exact solution vectors:
    acount = 1;
    linemod = 0;
    if Nrhs > 0,
       for i=1:Nrhs,
          for j=1:nrhsentries,
            fprintf(out_file,rformat,rhs(j));
            acount=acount+1;
            if rem(acount,Rhsperline) == linemod, fprintf(out_file,'\n'); end
          end
          if rem(acount, Rhsperline) ~= linemod,
            fprintf(out_file,'\n');
            linemod = rem((acount-1),Rhsperline);
          end
          rhs = rhs+nrhsentries;
          if Rhstype(2) == 'G',
            for j=1:nrhsentries,
              fprintf(out_file,rformat,guess(j));
              acount=acount+1;
              if rem(acount,Rhsperline) == linemod, fprintf(out_file,'\n'); end
            end
            if rem(acount,Rhsperline) ~= linemod,
              fprintf(out_file,'\n');
              linemod = rem((acount-1),Rhsperline);
            end
            guess = guess+nrhsentries;
          end
          if Rhstype(3) == 'X',
            for j=1:nrhsentries,
              fprintf(out_file,rformat,exact(j));
              acount=acount+1;
              if rem(acount,Rhsperline) == linemod, fprintf(out_file,'\n');end
            end
            if rem(acount,Rhsperline) ~= linemod,
              fprintf(out_file,'\n');
              linemod = rem((acount-1),Rhsperline);
            end
            exact = exact+nrhsentries;
          end
       end
    end

    end

    fclose(out_file);

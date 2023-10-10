%Hackmat, a HackRF interface for MATLAB
%
%Mikko Laakso
%mikko.t.laakso@aalto.fi
%
%Aalto University, School of Electrical Engineering, Department of
%Signal Processing and Acoustics
%
%System object that interfaces the C MEX HackRF interface


%single char commands passed to c code:
%e - enumerate()
%s - stream()
classdef HackRF < matlab.System
    properties %(Access = private)
      %setuppassed = 'uninitialized';
      SampleRate
      BandWidth
      CenterFrequency
      LnaGain
      VgaGain
    end
   
   methods
       function obj = HackRF(varargin)
            setProperties(obj,nargin,varargin{:});
       end 
      
       function enumerate(~)
           hackmat('e');
       end
       function printsettings(~)
           hackmat('z');
       end
   end
   
   methods(Access = protected)
       
       function validatePropertiesImpl(obj)
           if isempty(obj.VgaGain)
               obj.VgaGain=0;
           end
           if isempty(obj.LnaGain)
               obj.LnaGain=0;
           end
           if isempty(obj.SampleRate) 
               error('Property SampleRate needs to be specified');
           end
           if isempty(obj.SampleRate) 
               error('Property CenterFrequency needs to be specified');
           end 
       end
       
       
      function processTunedPropertiesImpl(obj)
         
      end
      
      function setupImpl(obj)
          hackmat('d',obj.SampleRate,obj.CenterFrequency,obj.LnaGain,obj.VgaGain);
      end
      
      
      function [y] =stepImpl(varargin)
          switch nargin
            case 1
                y=hackmat('r');
            case 2
                hackmat('t',varargin{2});
                y=1;
          end
      end
      
      function releaseImpl(~)
          hackmat('c');
          clear mex;
      end
   end
end

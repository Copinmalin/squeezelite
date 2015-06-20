<!DOCTYPE html>
<html>
<head>
 <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" >
 <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" >
 
  <title>
 squeezelite -
 
 
 lightweight headless squeezebox emulator - Google Project Hosting
 </title>
 
<!--[if IE]>
 <link type="text/css" rel="stylesheet" href="https://ssl.gstatic.com/codesite/ph/8599073060794244502/css/d_ie.css" >
<![endif]-->
 
</head>

 <td id="wikicontent" class="psdescription">
 <h1><a name="Headless_squeezebox_emulator_for_linux/osx/windows"></a>Headless squeezebox emulator for linux/osx/windows<a href="#Headless_squeezebox_emulator_for_linux/osx/windows" class="section_anchor"></a></h1><p>Squeezelite is a small headless squeezebox emulator for linux using alsa audio output and other platforms using portaudio.  It is aimed at supporting high quality audio including usb dac based output at multiple sample rates including 44.1/48/88.2/96/176.4/192k/352.8/384kHz. </p><p>The current version supports: <ul><li>built in pcm (wav/aiff) decode plus flac, mp3, ogg and aac via libFLAC, libmad/libmpg123, libvorbisfile, libfaad respectively if they are present on your machine. </li><li>includes support for wma and alac decode via the ffmpeg library if it is built with the -DFFMPEG build option.  (This is not included in binaries by default) </li><li>upsampling using the libsoxr resampling library if present on the machine.  This allows squeezelite to upsample the output to the highest sample rate supported by the output device.  It is optionally included in the build process.  (Resampling is not included in armv5te, armv6 and mips binaries in the download directory.)  Resampling support is enabled at compile time with the -DRESAMPLE build option. </li><li>export of audio data to jivelite on linux to support visualizations.  This is enabled with the -DVISEXPORT build option.  (This is not included in the binaries by default.)  </li><li>DSD playback via DOP capable DACs or via conversion to pcm.  This is enabled with the -DDSD build option and requires additional LMS server patches. </li></ul></p><p>Run &quot;./squeezelite -?&quot; for command line options included in the binary: </p><pre class="prettyprint">
Squeezelite v1.7, Copyright 2012-2015 Adrian Smith. See -t for license terms
Usage: ./squeezelite [options]
  -s &lt;server&gt;[:&lt;port&gt;]  Connect to specified server, otherwise uses autodiscovery to find server
  -o &lt;output device&gt;    Specify output device, default &quot;default&quot;, - = output to stdout
  -l                    List output devices
  -a &lt;b&gt;:&lt;p&gt;:&lt;f&gt;:&lt;m&gt;    Specify ALSA params to open output device, b = buffer time in ms or size in bytes, p = period count or size in bytes, f sample format (16|24|24_3|32), m = use mmap (0|1)
  -a &lt;f&gt;                Specify sample format (16|24|32) of output file when using -o - to output samples to stdout (interleaved little endian only)
  -b &lt;stream&gt;:&lt;output&gt;  Specify internal Stream and Output buffer sizes in Kbytes
  -c &lt;codec1&gt;,&lt;codec2&gt;  Restrict codecs to those specified, otherwise load all available codecs; known codecs: flac,pcm,mp3,ogg,aac,wma,alac,dsd (mad,mpg for specific mp3 codec)
  -d &lt;log&gt;=&lt;level&gt;      Set logging level, logs: all|slimproto|stream|decode|output, level: info|debug|sdebug
  -e &lt;codec1&gt;,&lt;codec2&gt;  Explicitly exclude native support of one or more codecs; known codecs: flac,pcm,mp3,ogg,aac,wma,alac,dsd (mad,mpg for specific mp3 codec)
  -f &lt;logfile&gt;          Write debug to logfile
  -m &lt;mac addr&gt;         Set mac address, format: ab:cd:ef:12:34:56
  -M &lt;modelname&gt;        Set the squeezelite player model name sent to the server (default: SqueezeLite)
  -n &lt;name&gt;             Set the player name
  -N &lt;filename&gt;         Store player name in filename to allow server defined name changes to be shared between servers (not supported with -n)
  -p &lt;priority&gt;         Set real time priority of output thread (1-99)
  -P &lt;filename&gt;         Store the process id (PID) in filename
  -r &lt;rates&gt;[:&lt;delay&gt;]  Sample rates supported, allows output to be off when squeezelite is started; rates = &lt;maxrate&gt;|&lt;minrate&gt;-&lt;maxrate&gt;|&lt;rate1&gt;,&lt;rate2&gt;,&lt;rate3&gt;; delay = optional delay switching rates in ms
  -R -u [params]        Resample, params = &lt;recipe&gt;:&lt;flags&gt;:&lt;attenuation&gt;:&lt;precision&gt;:&lt;passband_end&gt;:&lt;stopband_start&gt;:&lt;phase_response&gt;,
                         recipe = (v|h|m|l|q)(L|I|M)(s) [E|X], E = exception - resample only if native rate not supported, X = async - resample to max rate for device, otherwise to max sync rate
                         flags = num in hex,
                         attenuation = attenuation in dB to apply (default is -1db if not explicitly set),
                         precision = number of bits precision (NB. HQ = 20. VHQ = 28),
                         passband_end = number in percent (0dB pt. bandwidth to preserve. nyquist = 100%),
                         stopband_start = number in percent (Aliasing/imaging control. &gt; passband_end),
                         phase_response = 0-100 (0 = minimum / 50 = linear / 100 = maximum)
  -D [delay]            Output device supports DSD over PCM (DoP), delay = optional delay switching between PCM and DoP in ms
  -v                    Visualiser support
  -z                    Daemonize
  -t                    License terms
  -?                    Display this help text

Build options: LINUX ALSA EVENTFD RESAMPLE FFMPEG VISEXPORT DSD
</pre><hr/><p>Build Instructions <a href="/p/squeezelite/wiki/BuildInstructions">BuildInstructions</a> </p><hr/><p>Binary Files are now available from the companion project squeezelite-downloads: </p><ul><li>Linux armv5te - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-armv5te" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-armv5te</a> </li><li>Linux armv6 - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-armv6" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-armv6</a> </li><li>Linux armv6hf - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-armv6hf" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-armv6hf</a> </li><li>Linux intel 32 bit - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-i386" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-i386</a> </li><li>Linux intel 64 bit - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-x86-64" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-x86-64</a> </li><li>Linux mips ar71xx - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-mips-ar71xx" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-mips-ar71xx</a> </li><li>OSX 32/64 bit - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-osx" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-osx</a> </li><li>OSX 32 bit - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-osx-i386" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-osx-i386</a> </li><li>Windows - <a href="http://squeezelite-downloads.googlecode.com/git/squeezelite-win.zip" rel="nofollow">http://squeezelite-downloads.googlecode.com/git/squeezelite-win.zip</a> </li></ul>
 </td>
 </tr>
</table>

 </body>
</html>


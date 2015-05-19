Gem::Specification.new do |spec|
	spec.name = 'giflib'
	spec.version = '0.11'
	spec.summary = 'Summary'
	spec.description = 'Description'
	spec.email = 'ignore this'
	spec.homepage = 'http://myapp.com'
	spec.author = 'me'
#	spec.bindir = 'bin'
#	spec.executable = 'exec.rb'
	spec.files = Dir['ext/**/*.*'] + Dir['ext/**/*'] + Dir['test/**/*'] + Dir['*']
	spec.platform = Gem::Platform::RUBY
	spec.require_paths = [ 'ext' ]
	spec.extensions = Dir['ext/**/extconf.rb']
end


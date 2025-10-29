import Editor from '@monaco-editor/react';

type CodeEditorProps = {
  language: 'cpp' | 'python' | 'java';
  value: string;
  onChange: (v: string | undefined) => void;
  /** Altura opcional (ej. "300px", "50vh"). Por defecto 300px. */
  height?: string;
  className?: string;
};

export default function CodeEditor({
  language,
  value,
  onChange,
  height = '300px',
  className,
}: CodeEditorProps) {
  return (
    <div className={className} style={{ border: '1px solid #eee', borderRadius: 8, overflow: 'hidden' }}>
      <Editor
        height={height}
        language={language}
        value={value}
        onChange={onChange}
        options={{
          minimap: { enabled: false },
          fontSize: 14,
          scrollBeyondLastLine: false,
          automaticLayout: true,
        }}
      />
    </div>
  );
}

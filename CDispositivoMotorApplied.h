// "$Date: 2018-03-23 13:11:01 -0300 (sex, 23 mar 2018) $"
// "$Author: fares $"
// "$Revision: 4195 $"

#ifndef CMOTORAPPLIED_H
#define CMOTORAPPLIED_H

#include "IDispositivoMotor.h"
#include <QList>
#include <QVector>
#include <QTimer>

#ifdef SEPARADOR_COMUNICACAO
#undef SEPARADOR_COMUNICACAO
#endif
#define SEPARADOR_COMUNICACAO '\x0D'

class CDispositivoMotorApplied : public IDispositivoMotor {
	Q_OBJECT
public:
	explicit CDispositivoMotorApplied(CDadosConfiguracao * configuracao, QObject *parent = nullptr);
	~CDispositivoMotorApplied();
	void restringirListaPortas();
	bool inicializar();
	void finalizar();
	bool valorEntradaFimCurso(QString *mensagem = nullptr);
	bool valorEntradaPressostato(QString *mensagem = nullptr);
	void efetivarConfiguracao();
	void tratarResposta(); // QByteArray bufferEntrada, QByteArray bufferSaida);
	void reiniciarFabricaDispositivo();
	void configurarParaTipo(IDispositivoMotor::tipo tipo, QString porta = "");

	void zerarAlarme();
	void zerarPassos();
	void ajustarPosicao(qint32 posicao_uM);
	void ajustarPosicaoInicialReferencia();
	void moveContinuoNegativo(qint32 limite_uM = 2147483647);	// este é o maior número para um qint32 e serve como código para se decidir se realmente usa o limite ou se é contínuo mesmo;
	void moveContinuoPositivo(qint32 limite_uM = 2147483647);	// este é o maior número para um qint32 e serve como código para se decidir se realmente usa o limite ou se é contínuo mesmo;
	void moveContinuoInterrompe();
	void movePosicaoPassosAbsoluta(qint32 posPassosDestino);
	void movePosicaoPassosRelativa(qint32 posPassosDestino);
	void movePosicaoEncoderAbsoluta(qint32 posEncoder);
	void movePosicaoEncoderRelativa(qint32 posEncoder);
	void movePosicaoUmAbsoluta(qint32 posUm);
	void movePosicaoUmRelativa(qint32 posUm);
	void ativaSaida(int ordem, bool estado);
	void buscarFimCurso(IDispositivoMotor::sentido direcao = IDispositivoMotor::Direita, IDispositivoMotor::borda borda = IDispositivoMotor::Subida , int distanciaDesaceleracao = 6350);
	void modoTeste(bool modo, QString porta = "");
	void liberarDispositivo(IDispositivoMotor::tipo t);

	void usarVelocidadeAlta();
	void usarVelocidadeBaixa();
	void usarVelocidadeMedia();
	void usarVelocidadeMaxima();
	void usarVelocidadeMinima();
	void usarVelocidade(qint32 valor);
	void travarFreio(bool estado);
	void travarElevador(bool estado);

	void iniciarTesteVaiVolta(qint32 quant_uM);
	void finalizarTesteVaiVolta();

protected:
	void enviaSolicitacaoEstado();

public
Q_SLOTS:
	void slotRestringirListaPortasContinua();
	void slotReabrirPorta();
	void slotMudancaTipoDetectado(IDispositivoMotor::tipo tipoAntigo, IDispositivoMotor::tipo tipoNovo, QString porta);
	void slotReceberPosicaoDestinoAbsoluta(IDispositivoMotor::tipo tipoDispositivo, qint32 posicaoUmDestinoUm);
	void slotIniciarMovimentoContinuo(IDispositivoMotor::tipo tipoDispositivo, IDispositivoMotor::sentido s, IDispositivoMotor::intensidade v, qint32 limiteSuperior, qint32 limiteInferior);
	void slotPararMovimentoContinuo(IDispositivoMotor::tipo tipoDispositivo);
	void slotModoTesteTimerTimeout();
	void slotVelocidadeIntensidade(IDispositivoMotor::tipo tipoDispositivo, IDispositivoMotor::intensidade vel);
	void slotRespostaComandoTesteVaiVolta(IDispositivoMotor::tipo tipoDispositivo, qint32 pos);
	void slotObservaPonto();

#if defined(_DEBUG) || defined(_DETALHE_MOTOR_APPLIED)
public:
Q_SIGNALS:
	void Log(int,QString,QString);
#endif

public:
	enum etapaTesteVaiVolta {
		EsperaTerminar,
		Vai,
		Volta,
		Observa
	};


private:
	QByteArray		mComandoDefeituoso;
	qint32			mVelocidadeAntiga;
	qint32			mVaiVoltaQtd;
	QTimer			mTimerProssegue;
	volatile int	mSeletorEstado;
	bool			mRestringindoPortas;
	bool			mConfigurandoTipo;


	bool mProgramando;

	etapaTesteVaiVolta	mEtapaTesteVaiVolta;
	etapaTesteVaiVolta	mEtapaTesteVaiVoltaAnterior;
	QTimer				*mTimerVaiVolta;

	/*
	bool mAlarmeLimitePosicao;
	bool mAlarmeLimiteHorario;
	bool mAlarmeLimiteContraHorario;
	bool mAlarmeSobreTemperatura;
	bool mAlarmeTensaoInterna;
	bool mAlarmeSobreTensao;
	bool mAlarmeBaixaTensao;
	bool mAlarmeSobreCorrente;
	bool mAlarmeMotorAberto;
	bool mAlarmeDefeitoEncoder;
	bool mAlarmeErroComunicacao;
	bool mAlarmeErroMemoria;
	bool mAlarmeImpossivelMover;
	bool mAlarmeNaoUsado1;
	bool mAlarmeSegmentoQapagado;
	bool mAlarmeNaoUsado2;

	bool mEstadoMotorHabilitado;
	bool mEstadoAmostrando;
	bool mEstadoErro;	// verificar alarmes
	bool mEstadoEmPosicao;
	bool mEstadoMovendo;
	bool mEstadoEmJog;
	bool mEstadoParando;
	bool mEstadoAguardandoEntrada;
	bool mEstadoSalvandoParametro;
	bool mEstadoAlarme;	// verificar alarmes
	bool mEstadoLocalizandoHome;
	bool mEstadoAguardandoTempo;
	bool mEstadoAssistenteRodando;
	bool mEstadoVerificandoEncoder;
	bool mEstadoRodandoProgramaQ;
	bool mEstadoInicializando;

	bool mSaida_00;
	bool mSaida_01;
	bool mSaida_02;
	bool mSaida_03;
	bool mSaida_04;
	bool mSaida_05;
	bool mSaida_06;
	bool mSaida_07;
	bool mSaida_08;
	bool mSaida_09;
	bool mSaida_10;
	bool mSaida_11;
	bool mSaida_12;
	bool mSaida_13;
	bool mSaida_14;
	bool mSaida_15;

	bool mEntrada_X1_STEP;
	bool mEntrada_X2_DIR;
	bool mEntrada_X3_Enable;
	bool mEntrada_X4_AlarmReset;
	bool mEntrada_X5;
	bool mEntrada_X6_CCWlimit;
	bool mEntrada_X7_CWlimit;
	bool mEntrada_X0_EncoderZ;
	*/

};

#endif // CMOTORAPPLIED_H

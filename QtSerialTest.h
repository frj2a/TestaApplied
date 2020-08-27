// "$Date: 2018-03-23 13:11:01 -0300 (sex, 23 mar 2018) $"
// "$Author: fares $"
// "$Revision: 4195 $"

#ifndef QTSERIALTEST_H
#define QTSERIALTEST_H

class LogNetSender;

#include "ui_QtSerialTest.h"
#include "IDispositivoMotor.h"
#include <QThread>
#include <QTimer>

class QtSerialTest : public QWidget, private Ui::QtSerialTest {
	Q_OBJECT

public:
	explicit QtSerialTest(QWidget *pai = 0);
	~QtSerialTest();

protected:
	void changeEvent(QEvent *e);

private:
	void habilitarInterface(bool estado = true, QPushButton * excessao = NULL);

private:
	char								mArgC;
	char**								mArgV;
	QThread								mThreadLog;
	LogNetSender						* mLogSender;
	IDispositivoMotor					* mDispositivo;
	IDispositivoMotor::tipo				mTipoDispositivoEmUso;
	QHash<QString, IDispositivoMotor *>	mDispositivos;
	bool								mEditandoMoverEncoder;
	bool								mEditandoMoverPassos;
	bool								mEmMovimento;
	bool								mMovendoEncoder;
	qint32								mPosEncoder;
	qint32								mPosPassos;
	qint32								mPosPassosDesejada;
	qint32								mPosEncoderDesejada;
	bool								mPortaAberta;
	bool								mPortaAcabouAbrirVerificaSaidas;
	bool								mMensagensAtualizadas;
	bool								mEmTesteVaiVolta;
	QTimer								mTimerFaltaResposta;

#if defined(_DETALHE_DISPOSITIVO_MOTOR) || defined(_DETALHE_MOTOR_APPLIED)
public:
Q_SIGNALS:
	void Log(int,QString,QString,QString);
#endif

public
Q_SLOTS:
	void on_pbAbrirPorta_clicked(bool estado);
	void on_pbEnviar_clicked();
	void on_leComando_returnPressed()			{	on_pbEnviar_clicked();			}
	void on_cbPortas_currentIndexChanged(int item);
	void on_pbMoverEncoder_clicked();
	void on_pbZeraEncoder_clicked();
	void on_pbMoverPassos_clicked();
	void on_pbZeraPassos_clicked();
	void on_pbLimpaAlarme_clicked();
	void on_pbPositivo_pressed();
	void on_pbPositivo_released();
	void on_pbNegativo_pressed();
	void on_pbNegativo_released();
	void on_pbFimCursoPositivo_clicked();
	void on_pbFimCursoNegativo_clicked();
	void on_lePosEncoder_selectionChanged();
	void on_lePosEncoder_returnPressed()		{	on_pbMoverEncoder_clicked();	}
	void on_lePosPassos_selectionChanged();
	void on_lePosPassos_returnPressed()			{	on_pbMoverPassos_clicked();		}
	void on_rbVelocidadeAlta_toggled(bool checked);
	void on_rbVelocidadeMedia_toggled(bool checked);
	void on_rbVelocidadeBaixa_toggled(bool checked);
	void on_twSaidas_clicked(QModelIndex index);
	void on_pbTesteVaiVolta_clicked(bool estado);

	void slotDispositivosEncontrados();
	void slotPosicao_uMAtual(IDispositivoMotor::tipo tipoMotor, qint32 posEnc);
	void slotPosicaoPassosAtual(IDispositivoMotor::tipo tipoMotor, qint32 posPas);
	void slotEstado(IDispositivoMotor::tipo tipoMotor, quint16 estado);
	void slotDispositivoEmMovimento(IDispositivoMotor::tipo tipoMotor, bool estado);
	void slotAlarme(IDispositivoMotor::tipo tipoMotor, quint16 alarme);
	void slotSaida(IDispositivoMotor::tipo tipoMotor, quint16 saidas);
	void slotEntrada(IDispositivoMotor::tipo tipoMotor, quint16 entradas);
	void slotEstadoComunicacao(IDispositivoMotor::tipo tipoMotor, bool estado);
	void slotInicializado(IDispositivoMotor::tipo tipoMotor);
	void slotRespostaAComandoUsuario(IDispositivoMotor::tipo tipoMotor, QString resposta);

	void slotSemRespostaHabilitaInterface();
};

#endif // QTSERIALTEST_H
